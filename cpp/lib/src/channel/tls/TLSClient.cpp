/*
 * Copyright 2013-2022 Step Function I/O, LLC
 * Modified 2024-2026 f0rw4rd (experimental fork)
 *
 * Licensed to Green Energy Corp (www.greenenergycorp.com) and Step Function I/O
 * LLC (https://stepfunc.io) under one or more contributor license agreements.
 * See the NOTICE file distributed with this work for additional information
 * regarding copyright ownership. Green Energy Corp and Step Function I/O LLC license
 * this file to you under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "channel/tls/TLSClient.h"

#include "channel/SocketHelpers.h"

#include "opendnp3/logging/LogLevels.h"

#include <utility>

namespace opendnp3
{

TLSClient::TLSClient(const Logger& logger,
                     const std::shared_ptr<exe4cpp::StrandExecutor>& executor,
                     std::string adapter,
                     const TLSConfig& config,
                     std::error_code& ec)
    : logger(logger),
      condition(logger),
      executor(executor),
      adapter(std::move(adapter)),
      ctx(logger, false, config, ec),
      verifyCallback(config.verifyCallback),
      resolver(*executor->get_context())
{
}

bool TLSClient::Cancel()
{
    if (this->canceled)
    {
        return false;
    }

    std::error_code ec;
    resolver.cancel();
    this->canceled = true;
    return true;
}

bool TLSClient::BeginConnect(const IPEndpoint& remote, const connect_callback_t& callback)
{
    if (canceled)
        return false;

    auto stream
        = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(*this->executor->get_context(), this->ctx.value);

    auto verify = [self = shared_from_this()](bool preverified, asio::ssl::verify_context& ctx) -> bool {
        self->LogVerifyCallback(preverified, ctx);
        if (self->verifyCallback)
        {
            X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
            if (!cert)
            {
                SIMPLE_LOG_BLOCK(self->logger, flags::ERR,
                                 "verifyCallback: X509_STORE_CTX_get_current_cert returned NULL");
                return false;
            }
            int depth = X509_STORE_CTX_get_error_depth(ctx.native_handle());
            char subjectName[512];
            X509_NAME_oneline(X509_get_subject_name(cert), subjectName, 512);
            unsigned char* der = nullptr;
            int derLen = i2d_X509(cert, &der);
            std::string certDER;
            if (der && derLen > 0)
            {
                certDER.assign(reinterpret_cast<const char*>(der), derLen);
            }
            OPENSSL_free(der);

            // Snapshot error message inside try block where the GIL may still
            // be held by the caller (pybind11 wrapper).  After stack unwinding
            // the GIL is released, so we must NOT call ex.what() or any Python
            // API in the catch handler — only use the pre-captured string.
            std::string errorMsg;
            try
            {
                return self->verifyCallback(preverified, depth, std::string(subjectName), certDER);
            }
            catch (const std::exception& ex)
            {
                // Capture the message NOW while the GIL may still be held
                // by the pybind11 lambda that wraps the Python callback.
                // NOTE: for py::error_already_set, what() fetches and clears
                // the Python error — safe here because pybind11's
                // gil_scoped_acquire is still alive at the throw site.
                // However, stack unwinding destroys it before we reach this
                // catch block, so what() is only safe for non-Python exceptions.
                // Use a fixed message to be safe in all cases.
                errorMsg = "verifyCallback threw exception";
            }
            catch (...)
            {
                errorMsg = "verifyCallback threw unknown exception";
            }
            // Log AFTER the catch — errorMsg is a plain std::string, no
            // Python API calls needed.  If the log handler is a Python
            // trampoline, FORMAT_LOG_BLOCK might call into Python without
            // the GIL, but that is a pre-existing issue with all logging
            // on ASIO threads and is not specific to this code path.
            SIMPLE_LOG_BLOCK(self->logger, flags::ERR, errorMsg.c_str());
            return false;
        }
        return preverified;
    };

    std::error_code ec;
    stream->set_verify_callback(verify, ec);

    if (ec)
    {
        auto cb = [self = shared_from_this(), callback, stream, ec] {
            if (!self->canceled)
            {
                callback(self->executor, stream, ec);
            }
        };

        this->executor->post(cb);
        return true;
    }

    SocketHelpers::BindToLocalAddress<asio::ip::tcp>(this->adapter, 0, stream->lowest_layer(), ec);

    if (ec)
    {
        auto cb = [self = shared_from_this(), callback, stream, ec] {
            if (!self->canceled)
            {
                callback(self->executor, stream, ec);
            }
        };

        this->executor->post(cb);
        return true;
    }

    const auto address = asio::ip::address::from_string(remote.address, ec);
    auto self = this->shared_from_this();
    if (ec)
    {
        // Try DNS resolution instead
        auto cb = [self, callback, stream](const std::error_code& ec, asio::ip::tcp::resolver::iterator endpoints) {
            self->HandleResolveResult(callback, stream, endpoints, ec);
        };

        std::stringstream portstr;
        portstr << remote.port;

        resolver.async_resolve(asio::ip::tcp::resolver::query(remote.address, portstr.str()), executor->wrap(cb));

        return true;
    }

    asio::ip::tcp::endpoint remoteEndpoint(address, remote.port);
    auto cb = [self, stream, callback](const std::error_code& ec) { self->HandleConnectResult(callback, stream, ec); };

    stream->lowest_layer().async_connect(remoteEndpoint, executor->wrap(cb));
    return true;
}

void TLSClient::LogVerifyCallback(bool preverified, asio::ssl::verify_context& ctx)
{
    const int MAX_SUBJECT_NAME = 512;

    int depth = X509_STORE_CTX_get_error_depth(ctx.native_handle());

    // lookup the subject name
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    char subjectName[MAX_SUBJECT_NAME];
    if (cert)
    {
        X509_NAME_oneline(X509_get_subject_name(cert), subjectName, MAX_SUBJECT_NAME);
    }
    else
    {
        snprintf(subjectName, MAX_SUBJECT_NAME, "(no certificate)");
    }

    if (preverified)
    {
        FORMAT_LOG_BLOCK(this->logger, flags::INFO, "Verified certificate at depth: %d subject: %s", depth,
                         subjectName);
    }
    else
    {
        const int err = X509_STORE_CTX_get_error(ctx.native_handle());
        FORMAT_LOG_BLOCK(this->logger, flags::WARN, "Error verifying certificate at depth: %d subject: %s error: %d:%s",
                         depth, subjectName, err, X509_verify_cert_error_string(err));
    }
}

void TLSClient::HandleResolveResult(const connect_callback_t& callback,
                                    const std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>>& stream,
                                    const asio::ip::tcp::resolver::iterator& endpoints,
                                    const std::error_code& ec)
{
    if (ec)
    {
        if (!this->canceled)
        {
            callback(this->executor, stream, ec);
        }
    }
    else
    {
        // attempt a connection to each endpoint in the iterator until we connect
        auto cb = [self = this->shared_from_this(), callback, stream](const std::error_code& ec,
                                                                      asio::ip::tcp::resolver::iterator endpoints) {
            self->HandleConnectResult(callback, stream, ec);
        };

        asio::async_connect(stream->lowest_layer(), endpoints, this->condition, this->executor->wrap(cb));
    }
}

void TLSClient::HandleConnectResult(const connect_callback_t& callback,
                                    const std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>>& stream,
                                    const std::error_code& ec)
{
    if (ec)
    {
        if (!this->canceled)
        {
            callback(this->executor, stream, ec);
        }
    }
    else
    {
        auto cb = [self = shared_from_this(), callback, stream](const std::error_code& ec) {
            if (!self->canceled)
            {
                callback(self->executor, stream, ec);
            }
        };

        stream->async_handshake(asio::ssl::stream_base::client, executor->wrap(cb));
    }
}

} // namespace opendnp3

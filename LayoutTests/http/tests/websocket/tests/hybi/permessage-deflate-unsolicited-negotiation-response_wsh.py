# Copyright 2013, Google Inc.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


import urlparse
from mod_pywebsocket.extensions import PerMessageDeflateExtensionProcessor


def _get_permessage_deflate_extension_processor(request):
    for extension_processor in request.ws_extension_processors:
        if isinstance(extension_processor, PerMessageDeflateExtensionProcessor):
            return extension_processor
    return None


def web_socket_do_extra_handshake(request):
    processor = _get_permessage_deflate_extension_processor(request)
    # Remove extension processors other than PerMessageDeflateProcessor
    # to avoid conflict.
    request.ws_extension_processors = [processor]
    if not processor:
        return
    # Insert fake s2c_... parameters to make the server create
    # a negotiation response including the parameters.
    processor._request.add_parameter('s2c_no_context_takeover', None)
    processor._request.add_parameter('s2c_max_window_bits', '15')


def web_socket_transfer_data(request):
    pass


# vi:sts=4 sw=4 et

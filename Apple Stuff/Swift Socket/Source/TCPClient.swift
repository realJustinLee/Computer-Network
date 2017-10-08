//
//  TCPClient.swift
//  Swift Socket
//
//  Created by 李欣 on 2017/10/8.
//  Copyright © 2017年 李欣. All rights reserved.
//

///
///  Copyright (c) <2017>, Lixin
///  All rights reserved.
///
///  Redistribution and use in source and binary forms, with or without
///  modification, are permitted provided that the following conditions are met:
///  1. Redistributions of source code must retain the above copyright
///  notice, this list of conditions and the following disclaimer.
///  2. Redistributions in binary form must reproduce the above copyright
///  notice, this list of conditions and the following disclaimer in the
///  documentation and/or other materials provided with the distribution.
///  3. All advertising materials mentioning features or use of this software
///  must display the following acknowledgement:
///  This product includes software developed by Lixin.
///  4. Neither the name of the Lixin nor the
///  names of its contributors may be used to endorse or promote products
///  derived from this software without specific prior written permission.
///
///  THIS SOFTWARE IS PROVIDED BY Lixin ''AS IS'' AND ANY
///  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///  DISCLAIMED. IN NO EVENT SHALL Lixin BE LIABLE FOR ANY
///  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
///  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
///  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
///  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
///  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
///  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///
///  Contact: JustinDellAdam@live.com
///

import Foundation

@_silgen_name("srcTcpSocket_connect") private func TcpSocket_connect(_ host: UnsafePointer<Byte>, port: Int32, timeout: Int32) -> Int32

@_silgen_name("srcTcpSocket_close") private func TcpSocket_close(_ fd: Int32) -> Int32

@_silgen_name("srcTcpSocket_send") private func TcpSocket_send(_ fd: Int32, buff: UnsafePointer<Byte>, len: Int32) -> Int32

@_silgen_name("srcTcpSocket_pull") private func TcpSocket_pull(_ fd: Int32, buff: UnsafePointer<Byte>, len: Int32, timeout: Int32) -> Int32

@_silgen_name("srcTcpSocket_listen") private func TcpSocket_listen(_ address: UnsafePointer<Int8>, port: Int32) -> Int32

@_silgen_name("srcTcpSocket_accept") private func TcpSocket_accept(_ on_socketFD: Int32, remoteIP: UnsafePointer<Int8>, remotePort: UnsafePointer<Int32>, timeout: Int32) -> Int32

@_silgen_name("srcTcpSocket_port") private func TcpSocket_port(_ fd: Int32) -> Int32

open class TCPClient: Socket {
    /*
     * 连接到服务器
     * 用 message 返回成败
     */
    open func connect(timeout: Int) -> Result {
        let res: Int32 = TcpSocket_connect(self.address, port: Int32(self.port), timeout: Int32(timeout))
        if res > 0 {
            self.fd = res
            return .success
        } else {
            switch res {
            case -1:
                return .failure(SocketError.queryFailed)
            case -2:
                return .failure(SocketError.connectionClosed)
            case -3:
                return .failure(SocketError.connectionTimeout)
            default:
                return .failure(SocketError.unknownError)
            }
        }
    }

    /*
     * 关闭套接字
     * 用 message 返回成败
     */
    open func close() {
        guard let fd = self.fd else {
            return
        }
        _ = TcpSocket_close(fd)
        self.fd = nil
    }

    /*
     * 发送数据
     * 用 message 返回成败
     */
    open func send(data: [Byte]) -> Result {
        guard let fd = self.fd else {
            return .failure(SocketError.connectionClosed)
        }

        let sendSize: Int32 = TcpSocket_send(fd, buff: data, len: Int32(data.count))
        if Int(sendSize) == data.count {
            return .success
        } else {
            return .failure(SocketError.unknownError)
        }
    }

    /*
     * 发送字符串
     * 用 message 返回成败
     */
}
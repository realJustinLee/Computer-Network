//
//  UDPBundle.swift
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

@_silgen_name("srcUdpSocket_server") func UdpSocket_server(_ host: UnsafePointer<Int8>, port: Int32) -> Int32

@_silgen_name("srcUdpSocket_receive") func UdpSocket_receive(_ fd: Int32, buff: UnsafePointer<Byte>, len: Int32, ip: UnsafePointer<Int8>, port: UnsafePointer<Int32>) -> Int32

@_silgen_name("srcUdpSocket_close") func UdpSocket_close(_ fd: Int32) -> Int32

@_silgen_name("srcUdpSocket_client") func UdpSocket_client() -> Int32

@_silgen_name("srcUdpSocket_get_server_ip") func UdpSocket_get_server_ip(_ host: UnsafePointer<Int8>, ip: UnsafePointer<Int8>) -> Int32

@_silgen_name("srcUdpSocket_sent_to") func UdpSocket_sent_to(_ fd: Int32, buff: UnsafePointer<Byte>, len: Int32, ip: UnsafePointer<Int8>, port: Int32) -> Int32

@_silgen_name("enable_broadcast") func enable_broadcast(_ fd: Int32)

open class UDPClient: Socket {
    public override init(address: String, port: Int32) {
        let remoteIP_buff: [Int8] = [Int8](repeating: 0x0, count: 16)
        let ret = UdpSocket_get_server_ip(address, ip: remoteIP_buff)
        guard let ip = String(cString: remoteIP_buff, encoding: String.Encoding.utf8), ret == 0 else {
            /// TODO: change to init?
            super.init(address: "", port: 0)
            return
        }

        super.init(address: ip, port: port)

        let fd: Int32 = UdpSocket_client()
        if fd > 0 {
            self.fd = fd
        }
    }

    /*
     * 发送数据
     * 用 message 返回成败
     */
    open func send(data: [Byte]) -> Result {
        guard let fd = self.fd else {
            return .failure(SocketError.connectionClosed)
        }

        let sendSize: Int32 = UdpSocket_sent_to(fd, buff: data, len: Int32(data.count), ip: self.address, port: Int32(self.port))
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
    open func send(string: String) -> Result {
        guard let fd = self.fd else {
            return .failure(SocketError.connectionClosed)
        }

        let sendSize = UdpSocket_sent_to(fd, buff: string, len: Int32(strlen(string)), ip: address, port: port)
        if sendSize == Int32(strlen(string)) {
            return .success
        } else {
            return .failure(SocketError.unknownError)
        }
    }

    /*
     * 开启广播功能
     */
    open func enableBroadcast() {
        guard let fd: Int32 = self.fd else {
            return
        }

        enable_broadcast(fd)
    }

    /*
     * 发送 NSData
     * 用 message 返回成败
     */
    open func send(data: Data) -> Result {
        guard let fd = self.fd else {
            return .failure(SocketError.connectionClosed)
        }

        var buff = [Byte](repeating: 0x0, count: data.count)
        (data as NSData).getBytes(&buff, length: data.count)
        let sendSize = UdpSocket_sent_to(fd, buff: buff, len: Int32(data.count), ip: address, port: port)
        if sendSize == Int32(data.count) {
            return .success
        } else {
            return .failure(SocketError.unknownError)
        }
    }

    /*
     * 接收消息
     */
    /// TODO: Add multicast and broadcast
    open func recv(_ expectedLen: Int) -> ([Byte]?, String, Int) {
        guard let fd = self.fd else {
            return (nil, "no ip", 0)
        }
        var buff: [Byte] = [Byte](repeating: 0x0, count: expectedLen)
        var remoteIP_buff: [Int8] = [Int8](repeating: 0x0, count: 16)
        var remotePort: Int32 = 0
        let readLen: Int32 = UdpSocket_receive(fd, buff: buff, len: Int32(expectedLen), ip: &remoteIP_buff, port: &remotePort)
        let port: Int = Int(remotePort)
        let address = String(cString: remoteIP_buff, encoding: String.Encoding.utf8) ?? ""

        if readLen <= 0 {
            return (nil, address, port)
        }

        let data: [Byte] = Array(buff[0..<Int(readLen)])
        return (data, address, port)
    }

    open func close() {
        guard let fd = self.fd else {
            return
        }

        _ = UdpSocket_close(fd)
        self.fd = nil
    }

    /// TODO: Add multicast and broadcast
}

open class UDPServer: Socket {
    public override init(address: String, port: Int32) {
        super.init(address: address, port: port)

        let fd = UdpSocket_server(address, port: port)
        if fd > 0 {
            self.fd = fd
        }
    }

    /// TODO: Add multicast and broadcast
    open func recv(_ expectedLen: Int) -> ([Byte]?, String, Int) {
        if let fd = self.fd {
            var buff: [Byte] = [Byte](repeating: 0x0, count: expectedLen)
            var remoteIP_buff: [Int8] = [Int8](repeating: 0x0, count: 16)
            var remotePort: Int32 = 0
            let readLen: Int32 = UdpSocket_receive(fd, buff: buff, len: Int32(expectedLen), ip: &remoteIP_buff, port: &remotePort)
            let port: Int = Int(remotePort)
            var address = ""
            if let ip = String(cString: remoteIP_buff, encoding: String.Encoding.utf8) {
                address = ip
            }

            if readLen <= 0 {
                return (nil, address, port)
            }

            let readSegment = buff[0...Int(readLen - 1)]
            let data: [Byte] = Array(readSegment)
            return (data, address, port)
        }
        return (nil, "No IP provided", 0)
    }

    open func close() {
        guard let fd = self.fd else {
            return
        }

        _ = UdpSocket_close(fd)
        self.fd = nil
    }
}

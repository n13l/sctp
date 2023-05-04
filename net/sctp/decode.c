/*
 * The MIT License (MIT)            (SCTP) Stream Control Transmission Protocol
 *
 * Copyright (c) 2013 Daniel Kubec <niel@rtfm.cz>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <sys/compiler.h>
#include <sys/log.h>
#include <sys/cpu.h>
#include <bsd/array.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <crypto/crc.h>
#include <net/sctp/protocol.h>

/* O(1) time branchless access for zero-based array (0, 1, 2, ..., N − 2). */
STATIC_ARRAY_STREAMLINED(const char * const, sctp_chunk_types, "n/a",
	[SCTP_CHUNK_DATA]                         = "data",
	[SCTP_CHUNK_INIT]                         = "init",
	[SCTP_CHUNK_INIT_ACK]                     = "init ack",
	[SCTP_CHUNK_SACK]                         = "sack",
	[SCTP_CHUNK_HBREQ]                        = "heartbeat req",
	[SCTP_CHUNK_HBACK]                        = "heartbeat ack",
	[SCTP_CHUNK_ABORT]                        = "abort",
	[SCTP_CHUNK_SHUTDOWN]                     = "shutdown",
	[SCTP_CHUNK_SHUTDOWN_ACK]                 = "shutdown ack",
	[SCTP_CHUNK_ERROR]                        = "error",
	[SCTP_CHUNK_COOKIE_ECHO]                  = "cookie echo",
	[SCTP_CHUNK_COOKIE_ACK]                   = "cookie ack",
	[SCTP_CHUNK_ECNE]                         = "ecne",
	[SCTP_CHUNK_CWR]                          = "cwr",
	[SCTP_CHUNK_IETF_EXTENSION]               = "ietf extension"
);

STATIC_ARRAY_STREAMLINED(unsigned, chunk_type_offsets, 0,
	[SCTP_CHUNK_DATA] = offsetof(struct sctp_stat, chunk_data), 
	[SCTP_CHUNK_INIT] = offsetof(struct sctp_stat, chunk_init), 
);

static struct sctp_stat sctp_stat = {};

void
sctp_checksum(struct sctp_packet *msg, unsigned int len)
{
	msg->hdr.checksum = cpu_be32(crc32_hash((byte *)msg, len));
}

int
sctp_validate(struct sctp_packet *msg, unsigned int len)
{
	unsigned long crc32 = be32_cpu(msg->hdr.checksum);
	msg->hdr.checksum = 0L;
	return ((crc32 == crc32_hash((byte *)msg, len)) ? 1 : -1);
}

const char *
sctp_chunk_type_str(unsigned int id)
{
	return sctp_chunk_types_fetch_zb(id);
}

void
sctp_init(struct sctp *sctp, byte *pdu, u16 len)
{
	(*sctp) = (struct sctp) { .pkt = (struct sctp_packet *)pdu, .len = len};
}

int
sctp_decode(struct sctp_packet *msg, unsigned int len)
{
	trace3("pdu len=%d, spost=%d, dport=%d", len, 
	         get_u16_be(&msg->hdr.src_port),
	         get_u16_be(&msg->hdr.dst_port));

	struct sctp_chunk *c = (struct sctp_chunk *)msg + sizeof(*msg);
	const char *type = sctp_chunk_types_fetch_zb(c->type);
	unsigned offset = chunk_type_offsets(c->type);
	FIELD_INC_AT(&sctp_stat, offset, unsigned long long);

	trace3("chunk id=%d, flags=%d, len=%d type=%s",
	       c->hdr.id, c->hdr.flags, c->hdr.len, type);
	trace3("tsn=%d, stream_id=%d, stream_sn=%d proto_id=%d",
	       c->tsn, c->stream_id, c->stream_sn, c->proto_id);

	return 0;
}

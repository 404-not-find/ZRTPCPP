/*
 * Copyright 2006 - 2018, Werner Dittmann
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Authors: Werner Dittmann <Werner.Dittmann@t-online.de>
 */

#include <libzrtpcpp/ZrtpPacketHello.h>

void ZrtpPacketHello::configureHello(ZrtpConfigure& config) {
    // The NumSupported* data is in ZrtpTextData.h 
    nHash = config.getNumConfiguredAlgos(HashAlgorithm);
    nCipher = config.getNumConfiguredAlgos(CipherAlgorithm);
    nPubkey = config.getNumConfiguredAlgos(PubKeyAlgorithm);
    nSas = config.getNumConfiguredAlgos(SasType);
    nAuth = config.getNumConfiguredAlgos(AuthLength);

    // length is fixed Header plus HMAC size (2*ZRTP_WORD_SIZE)
    int32_t length = sizeof(HelloPacket_t) + (2 * ZRTP_WORD_SIZE);
    length += nHash * ZRTP_WORD_SIZE;
    length += nCipher * ZRTP_WORD_SIZE;
    length += nPubkey * ZRTP_WORD_SIZE;
    length += nSas * ZRTP_WORD_SIZE;
    length += nAuth * ZRTP_WORD_SIZE;

    // Don't change order of this sequence
    oHash = sizeof(Hello_t);
    oCipher = oHash + (nHash * ZRTP_WORD_SIZE);
    oAuth = oCipher + (nCipher * ZRTP_WORD_SIZE);
    oPubkey = oAuth + (nAuth * ZRTP_WORD_SIZE);
    oSas = oPubkey + (nPubkey * ZRTP_WORD_SIZE);
    oHmac = oSas + (nSas * ZRTP_WORD_SIZE);         // offset to HMAC

    void* allocated = &data;
    memset(allocated, 0, sizeof(data));

    zrtpHeader = (zrtpPacketHeader_t *)&((HelloPacket_t *)allocated)->hdr;	// the standard header
    helloHeader = (Hello_t *)&((HelloPacket_t *)allocated)->hello;

    setZrtpId();

    // minus 1 for CRC size 
    setLength(length / ZRTP_WORD_SIZE);
    setMessageType((uint8_t*)HelloMsg);

    uint32_t lenField = nHash << 16U;
    for (int32_t i = 0; i < nHash; i++) {
        AlgorithmEnum& hash = config.getAlgoAt(HashAlgorithm, i);
        setHashType(i, (int8_t*)hash.getName());
    }

    lenField |= nCipher << 12U;
    for (int32_t i = 0; i < nCipher; i++) {
        AlgorithmEnum& cipher = config.getAlgoAt(CipherAlgorithm, i);
        setCipherType(i, (int8_t*)cipher.getName());
    }

    lenField |= nAuth << 8U;
    for (int32_t i = 0; i < nAuth; i++) {
        AlgorithmEnum& authLength = config.getAlgoAt(AuthLength, i);
        setAuthLen(i, (int8_t*)authLength.getName());
    }

    lenField |= nPubkey << 4U;
    for (int32_t i = 0; i < nPubkey; i++) {
        AlgorithmEnum& pubKey = config.getAlgoAt(PubKeyAlgorithm, i);
        setPubKeyType(i, (int8_t*)pubKey.getName());
    }

    lenField |= nSas;
    for (int32_t i = 0; i < nSas; i++) {
        AlgorithmEnum& sas = config.getAlgoAt(SasType, i);
        setSasType(i, (int8_t*)sas.getName());
    }
    *((uint32_t*)&helloHeader->flags) = zrtpHtonl(lenField);
}

ZrtpPacketHello::ZrtpPacketHello(const uint8_t *data) {

    zrtpHeader = (zrtpPacketHeader_t *)&((HelloPacket_t *)data)->hdr;	// the standard header
    helloHeader = (Hello_t *)&((HelloPacket_t *)data)->hello;

    // Force the isLengthOk() check to fail when we process the packet.
    if (getLength() < HELLO_FIXED_PART_LEN) {
        computedLength = 0;
        return;
    }

    uint32_t t = *((uint32_t*)&helloHeader->flags);
    uint32_t temp = zrtpNtohl(t);

    nHash = (temp & (0xfU << 16U)) >> 16U;
    nHash &= 0x7U;                              // restrict to max 7 algorithms
    nCipher = (temp & (0xfU << 12U)) >> 12U;
    nCipher &= 0x7U;
    nAuth = (temp & (0xfU << 8U)) >> 8U;
    nAuth &= 0x7U;
    nPubkey = (temp & (0xfU << 4U)) >> 4U;
    nPubkey &= 0x7U;
    nSas = temp & 0xfU;
    nSas &= 0x7U;

    // +2 : the MAC at the end of the packet
    computedLength = nHash + nCipher + nAuth + nPubkey + nSas + sizeof(HelloPacket_t)/ZRTP_WORD_SIZE + 2;

    oHash = sizeof(Hello_t);
    oCipher = oHash + (nHash * ZRTP_WORD_SIZE);
    oAuth = oCipher + (nCipher * ZRTP_WORD_SIZE);
    oPubkey = oAuth + (nAuth * ZRTP_WORD_SIZE);
    oSas = oPubkey + (nPubkey * ZRTP_WORD_SIZE);
    oHmac = oSas + (nSas * ZRTP_WORD_SIZE);         // offset to HMAC
}

int32_t ZrtpPacketHello::getVersionInt() {
    uint8_t* vp = getVersion();
    int32_t version = 0;

    if (isdigit(*vp) && isdigit(*vp+2)) {
        version = (*vp - '0') * 10;
        version += *(vp+2) - '0';
    }
    return version;
}

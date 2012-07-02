// EbitSim - Enhanced BitTorrent Simulation
// This program is under the terms of the Creative Commons
// Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
//
// You are free:
//
//    to Share - to copy, distribute and transmit the work
//    to Remix - to adapt the work
//
// Under the following conditions:
//
//    Attribution - You must attribute the work in the manner specified by the
//    author or licensor (but not in any way that suggests that they endorse you
//    or your use of the work).
//
//    Noncommercial - You may not use this work for commercial purposes.
//
//    Share Alike - If you alter, transform, or build upon this work, you may
//    distribute the resulting work only under the same or similar license to
//    this one.
//
// With the understanding that:
//
//    Waiver - Any of the above conditions can be waived if you get permission
//    from the copyright holder.
//
//    Public Domain - Where the work or any of its elements is in the public
//    domain under applicable law, that status is in no way affected by the
//    license.
//
//    Other Rights - In no way are any of the following rights affected by the
//    license:
//        - Your fair dealing or fair use rights, or other applicable copyright
//          exceptions and limitations;
//        - The author's moral rights;
//        - Rights other persons may have either in the work itself or in how
//          the work is used, such as publicity or privacy rights.
//
//    Notice - For any reuse or distribution, you must make clear to others the
//    license terms of this work. The best way to do this is with a link to this
//    web page. <http://creativecommons.org/licenses/by-nc-sa/3.0/>
//
// Author:
//     Pedro Manoel Fabiano Alves Evangelista <pevangelista@larc.usp.br>
//     Supervised by Prof Tereza Cristina M. B. Carvalho <carvalho@larc.usp.br>
//     Graduate Student at Escola Politecnica of University of Sao Paulo, Brazil
//
// Contributors:
//     Marcelo Carneiro do Amaral <mamaral@larc.usp.br>
//     Victor Souza <victor.souza@ericsson.com>
//
// Disclaimer:
//     This work is part of a Master Thesis developed by:
//        Pedro Evangelista, graduate student at
//        Laboratory of Computer Networks and Architecture
//        Escola Politecnica
//        University of Sao Paulo
//        Brazil
//     and supported by:
//        Innovation Center
//        Ericsson Telecomunicacoes S.A., Brazil.
//
// UNLESS OTHERWISE MUTUALLY AGREED TO BY THE PARTIES IN WRITING AND TO THE
// FULLEST EXTENT PERMITTED BY APPLICABLE LAW, LICENSOR OFFERS THE WORK AS-IS
// AND MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND CONCERNING THE WORK,
// EXPRESS, IMPLIED, STATUTORY OR OTHERWISE, INCLUDING, WITHOUT LIMITATION,
// WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
// NONINFRINGEMENT, OR THE ABSENCE OF LATENT OR OTHER DEFECTS, ACCURACY, OR THE
// PRESENCE OF ABSENCE OF ERRORS, WHETHER OR NOT DISCOVERABLE. SOME
// JURISDICTIONS DO NOT ALLOW THE EXCLUSION OF IMPLIED WARRANTIES, SO THIS
// EXCLUSION MAY NOT APPLY TO YOU.

/*
 * BitField.cc
 *
 *  Created on: Mar 25, 2010
 *      Author: pevangelista
 */

#include "BitField.h"

// helper function
namespace {
// private
std::set<int> getPiecesSet(boost::dynamic_bitset<> const& bitField) {
    std::set<int> ret;
    std::set<int>::iterator it = ret.begin();

    for (int i = 0; i < bitField.size(); ++i) {
        if (bitField.test(i)) {
            // efficient insert because pieces are inserted in order
            it = ret.insert(it, i);
        }
    }

    return ret;
}
}

BitField::BitField() :
        numberOfPieces(0), numberOfAvailable(0) {
}
/*!
 * @param numberOfPieces The number of pieces in the BitField.
 * @param pieceSize      The size of the pieces, in bytes.
 * @param seed           True if the BitField is a seeder BitField (all pieces present).
 */
BitField::BitField(int numberOfPieces, bool seed) :
        numberOfPieces(numberOfPieces),
            numberOfAvailable(seed ? numberOfPieces : 0),
            bitField(numberOfPieces) {
    if (seed) {
        this->bitField = this->bitField.flip();
    }
}

BitField::~BitField() {
}

void BitField::addPiece(int index) {
    // if piece was 0, then the number of pieces went up by one.
    if (!this->bitField.test(index)) {
        ++this->numberOfAvailable;
    }
    this->bitField[index] = 1;
}
/*!
 * The BitField is interesting if it has any piece not owned by this BitField.
 * BitField b must have the same number of pieces as this BitField.
 */
bool BitField::isBitFieldInteresting(BitField const& b) const {
    if (this->numberOfPieces != b.numberOfPieces) {
        throw std::invalid_argument("The BitField has the incorrect size");
    }

    return (~this->bitField & b.bitField).any();
}
/*!
 *
 */
std::set<int> BitField::getInterestingPieces(BitField const& b) const {
    if (this->numberOfPieces != b.numberOfPieces) {
        throw std::invalid_argument("The BitField has the incorrect size");
    }

    boost::dynamic_bitset<> interestingPieces = ~this->bitField & b.bitField;
    return getPiecesSet(interestingPieces);
}
std::set<int> BitField::getBitFieldPieces() const {
    return getPiecesSet(this->bitField);
}
bool BitField::hasPiece(int index) const {
    return this->bitField.test(index);
}
int BitField::getByteSize() const {
    return ((this->numberOfPieces / 8) + ((this->numberOfPieces % 8) ? 1 : 0));
}

size_t BitField::size() const {
    return this->bitField.size();
}

bool BitField::empty() const {
    return this->numberOfAvailable == 0;
}

bool BitField::full() const {
    return this->numberOfAvailable == this->numberOfPieces;
}

bool BitField::operator!=(BitField const& b) const {
    return this->bitField != b.bitField;
}

bool BitField::operator==(BitField const& b) const {
    return this->bitField == b.bitField;
}

std::string BitField::str() const {
    std::string strRep;
    boost::to_string(this->bitField, strRep);
    return strRep;
}
double BitField::getCompletedPercentage() const {
    return (double) this->numberOfAvailable * 100 / this->numberOfPieces;
}

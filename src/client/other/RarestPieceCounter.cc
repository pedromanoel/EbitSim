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
 * RarestPieceCounter.cc
 *
 *  Created on: May 4, 2010
 *      Author: pevangelista
 */

#include "RarestPieceCounter.h"
#include <list>
#include <vector>
#include <algorithm>

#include <random.h>
#include "BitField.h"

RarestPieceCounter::RarestPieceCounter() {
}
RarestPieceCounter::RarestPieceCounter(int numberOfPieces) :
        pieceCount(numberOfPieces, 0) {
}

RarestPieceCounter::~RarestPieceCounter() {
}

/*!
 * Each bit set in the BitField add one to the piece count.
 * Throw an std::out_of_range exception if the size of the pieceCount
 * vector is different from the size of the BitField.
 */
void RarestPieceCounter::addBitField(BitField const& bitField) {
    std::vector<bool> const& bitFieldVector = bitField.getBitFieldVector();
    if (this->pieceCount.size() != bitFieldVector.size()) {
        throw std::invalid_argument("The BitField has the incorrect size");
    }
    for (unsigned int i = 0; i < bitFieldVector.size(); ++i) {
        if (bitFieldVector[i]) {
            ++this->pieceCount[i];
        }
    }
}
/*!
 * Add one to the piece count with the passed index.
 */
void RarestPieceCounter::addPiece(int index) {
    if (index < 0 || (unsigned) index >= this->pieceCount.size()) {
        throw std::out_of_range("The piece index is out of bounds");
    }

    ++this->pieceCount[index];
}

int RarestPieceCounter::getPieceCount(int index) const {
    if (index < 0 || (unsigned) index >= this->pieceCount.size()) {
        throw std::out_of_range("The piece index is out of bounds");
    }
    return this->pieceCount.at(index);
}
/*! Invalid pieces will not be considered.
 * @param pieces .
 * @param numberOfPieces .
 */
std::vector<int> RarestPieceCounter::getRarestPieces(
        std::set<int> const& pieces, unsigned int numberOfPieces) const {
    if (numberOfPieces < 0) {
        throw std::invalid_argument("numberOfPieces must be bigger than zero");
    }
    if (pieces.empty()) {
        throw std::invalid_argument("pieces must not be empty");
    }

    std::vector<int> returnedPieces;

    // sets are ordered, so it is easy to find the minimum and maximum values.
    if ((*(pieces.begin()) < 0)
            || ((unsigned) *(pieces.rbegin()) >= this->pieceCount.size())) {
        throw std::logic_error(
                "There are pieces in the set that are outside the allowed range");
    }

    std::set<int>::const_iterator it = pieces.begin();
    // will store <count, index> pairs
    std::list<std::pair<int, int> > orderedList;

    // create list that will be ordered by piece quantity
    for (; it != pieces.end(); ++it) {
        orderedList.push_back(std::make_pair(this->pieceCount[*it], *it));
    }

    if (!orderedList.empty()) {
        // avoid reallocation when adding elements to the end of the vector
        returnedPieces.reserve(orderedList.size());
        // TODO Maybe, a way to make it more efficient would be to store the count groups
        //      and move the pieces from one group to another as their values changes

        // Sort according to the piece count, then index.
        // The pieces with the same count will be grouped in the sorted list
        orderedList.sort();

        std::list<std::pair<int, int> >::iterator it = orderedList.begin();
        int lastGroupCount = it->first;
        int retGroupBegin = 0;

        // random shuffle the groups of pieces with the same count
        for (int i = 0;
                it != orderedList.end()
                        && returnedPieces.size() < numberOfPieces; ++it, ++i) {
            // group changed
            if (lastGroupCount != it->first) {
                // shuffle the previous group
                std::random_shuffle(returnedPieces.begin() + retGroupBegin,
                        returnedPieces.begin() + i, intrand);
                // start a new group
                lastGroupCount = it->first;
                retGroupBegin = i;
            }

            returnedPieces.push_back(it->second);
        }

        // If all pieces were inserted, then it is necessary to shuffle the last group
        if (it == orderedList.end()) {
            std::random_shuffle(returnedPieces.begin() + retGroupBegin,
                    returnedPieces.end(), intrand);
        }
    }

    int front = returnedPieces.front();
    int back = returnedPieces.back();
    int size = returnedPieces.size();

    return returnedPieces;
}
/*!
 * Each bit set in the BitField subtract one to the piece count. The piece
 * count minimizes at zero.
 * Throw an std::out_of_range exception if the size of the pieceCount
 * vector is different from the size of the BitField.
 */
void RarestPieceCounter::removeBitField(BitField const& bitField) {
    std::vector<bool> const& bitFieldVector = bitField.getBitFieldVector();
    if (this->pieceCount.size() != bitFieldVector.size()) {
        throw std::invalid_argument("The BitField has the incorrect size");
    }
    for (unsigned int i = 0; i < bitFieldVector.size(); ++i) {
        if (bitFieldVector[i] && (pieceCount[i] > 0)) {
            --pieceCount[i];
        }
    }
}
size_t RarestPieceCounter::size() const {
    return this->pieceCount.size();
}

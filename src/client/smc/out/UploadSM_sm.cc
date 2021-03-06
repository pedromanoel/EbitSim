/*
 * ex: set ro:
 * DO NOT EDIT.
 * generated by smc (http://smc.sourceforge.net/)
 * from file : UploadSM_sm.sm
 */


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


#include "PeerWire_m.h"
#include "PeerWireThread.h"
#include "out/UploadSM_sm.h"

using namespace statemap;

// Static class declarations.
UploadMap_Stopped UploadMap::Stopped("UploadMap::Stopped", 0);
UploadMap_NotInterestingChoking UploadMap::NotInterestingChoking("UploadMap::NotInterestingChoking", 1);
UploadMap_InterestingChoking UploadMap::InterestingChoking("UploadMap::InterestingChoking", 2);
UploadMap_NotInterestingUnchoking UploadMap::NotInterestingUnchoking("UploadMap::NotInterestingUnchoking", 3);
UploadMap_InterestingUnchoking UploadMap::InterestingUnchoking("UploadMap::InterestingUnchoking", 4);

void UploadSMState::cancelMsg(UploadSMContext& context, CancelMsg const& msg)
{
    Default(context);
    return;
}

void UploadSMState::chokePeer(UploadSMContext& context)
{
    Default(context);
    return;
}

void UploadSMState::interestedMsg(UploadSMContext& context)
{
    Default(context);
    return;
}

void UploadSMState::notInterestedMsg(UploadSMContext& context)
{
    Default(context);
    return;
}

void UploadSMState::requestMsg(UploadSMContext& context, RequestMsg const& msg)
{
    Default(context);
    return;
}

void UploadSMState::sendPieceMsg(UploadSMContext& context)
{
    Default(context);
    return;
}

void UploadSMState::startMachine(UploadSMContext& context)
{
    Default(context);
    return;
}

void UploadSMState::stopMachine(UploadSMContext& context)
{
    Default(context);
    return;
}

void UploadSMState::unchokePeer(UploadSMContext& context)
{
    Default(context);
    return;
}

void UploadSMState::uploadRateTimer(UploadSMContext& context)
{
    Default(context);
    return;
}

void UploadSMState::Default(UploadSMContext& context)
{
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "TRANSITION   : Default"
            << std::endl;
    }

    throw (
        TransitionUndefinedException(
            context.getState().getName(),
            context.getTransition()));

    return;
}

void UploadMap_Default::sendPieceMsg(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::Default"
            << std::endl;
    }

    UploadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::Default::sendPieceMsg()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.printDebugMsg("sendPieceMsg out of place");
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::Default::sendPieceMsg()"
                << std::endl;
        }

        context.setState(endState);
    }
    catch (...)
    {
        context.setState(endState);
        throw;
    }

    return;
}

void UploadMap_Default::stopMachine(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::Default"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::Default::stopMachine()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.cancelUploadRequests();
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::Default::stopMachine()"
                << std::endl;
        }

        context.setState(UploadMap::Stopped);
    }
    catch (...)
    {
        context.setState(UploadMap::Stopped);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void UploadMap_Default::uploadRateTimer(UploadSMContext& context)
{

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::Default"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::Default::uploadRateTimer()"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : UploadMap::Default::uploadRateTimer()"
            << std::endl;
    }


    return;
}

void UploadMap_Stopped::Entry(UploadSMContext& context)

{
    PeerWireThread& ctxt(context.getOwner());

    ctxt.printDebugMsg("uploadSM - Stopped");
    ctxt.stopUploadTimers();
    return;
}

void UploadMap_Stopped::Default(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::Stopped"
            << std::endl;
    }

    UploadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::Stopped::Default()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.printDebugMsg("uploadSM - Thread terminated");
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::Stopped::Default()"
                << std::endl;
        }

        context.setState(endState);
    }
    catch (...)
    {
        context.setState(endState);
        throw;
    }

    return;
}

void UploadMap_Stopped::startMachine(UploadSMContext& context)
{

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::Stopped"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::Stopped::startMachine()"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : UploadMap::Stopped::startMachine()"
            << std::endl;
    }

    context.setState(UploadMap::NotInterestingChoking);
    (context.getState()).Entry(context);

    return;
}

void UploadMap_NotInterestingChoking::Entry(UploadSMContext& context)

{
    PeerWireThread& ctxt(context.getOwner());

    ctxt.printDebugMsg("uploadSM - NotInterestingChoking");
    return;
}

void UploadMap_NotInterestingChoking::interestedMsg(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::NotInterestingChoking"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::NotInterestingChoking::interestedMsg()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.setInterested(true);
        ctxt.addPeerToChoker();
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::NotInterestingChoking::interestedMsg()"
                << std::endl;
        }

        context.setState(UploadMap::InterestingChoking);
    }
    catch (...)
    {
        context.setState(UploadMap::InterestingChoking);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void UploadMap_NotInterestingChoking::unchokePeer(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::NotInterestingChoking"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::NotInterestingChoking::unchokePeer()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getUnchokeMsg());
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::NotInterestingChoking::unchokePeer()"
                << std::endl;
        }

        context.setState(UploadMap::NotInterestingUnchoking);
    }
    catch (...)
    {
        context.setState(UploadMap::NotInterestingUnchoking);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void UploadMap_InterestingChoking::Entry(UploadSMContext& context)

{
    PeerWireThread& ctxt(context.getOwner());

    ctxt.printDebugMsg("uploadSM - InterestingChoking");
    return;
}

void UploadMap_InterestingChoking::notInterestedMsg(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::InterestingChoking"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::InterestingChoking::notInterestedMsg()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.setInterested(false);
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::InterestingChoking::notInterestedMsg()"
                << std::endl;
        }

        context.setState(UploadMap::NotInterestingChoking);
    }
    catch (...)
    {
        context.setState(UploadMap::NotInterestingChoking);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void UploadMap_InterestingChoking::requestMsg(UploadSMContext& context, RequestMsg const& msg)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::InterestingChoking"
            << std::endl;
    }

    UploadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::InterestingChoking::requestMsg(RequestMsg const& msg)"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getChokeMsg());
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::InterestingChoking::requestMsg(RequestMsg const& msg)"
                << std::endl;
        }

        context.setState(endState);
    }
    catch (...)
    {
        context.setState(endState);
        throw;
    }

    return;
}

void UploadMap_InterestingChoking::unchokePeer(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::InterestingChoking"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::InterestingChoking::unchokePeer()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getUnchokeMsg());
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::InterestingChoking::unchokePeer()"
                << std::endl;
        }

        context.setState(UploadMap::InterestingUnchoking);
    }
    catch (...)
    {
        context.setState(UploadMap::InterestingUnchoking);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void UploadMap_NotInterestingUnchoking::Entry(UploadSMContext& context)

{
    PeerWireThread& ctxt(context.getOwner());

    ctxt.printDebugMsg("uploadSM - NotInterestingUnchoking");
    return;
}

void UploadMap_NotInterestingUnchoking::chokePeer(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::NotInterestingUnchoking"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::NotInterestingUnchoking::chokePeer()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getChokeMsg());
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::NotInterestingUnchoking::chokePeer()"
                << std::endl;
        }

        context.setState(UploadMap::NotInterestingChoking);
    }
    catch (...)
    {
        context.setState(UploadMap::NotInterestingChoking);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void UploadMap_NotInterestingUnchoking::interestedMsg(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::NotInterestingUnchoking"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::NotInterestingUnchoking::interestedMsg()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.setInterested(true);
        ctxt.callChokeAlgorithm();
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::NotInterestingUnchoking::interestedMsg()"
                << std::endl;
        }

        context.setState(UploadMap::InterestingUnchoking);
    }
    catch (...)
    {
        context.setState(UploadMap::InterestingUnchoking);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void UploadMap_InterestingUnchoking::Entry(UploadSMContext& context)

{
    PeerWireThread& ctxt(context.getOwner());

    ctxt.printDebugMsg("uploadSM - InterestingUnchoking");
    ctxt.startUploadTimers();
    return;
}

void UploadMap_InterestingUnchoking::Exit(UploadSMContext& context)

{
    PeerWireThread& ctxt(context.getOwner());

    ctxt.stopUploadTimers();
    ctxt.cancelUploadRequests();
    return;
}

void UploadMap_InterestingUnchoking::cancelMsg(UploadSMContext& context, CancelMsg const& msg)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::InterestingUnchoking"
            << std::endl;
    }

    UploadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::InterestingUnchoking::cancelMsg(CancelMsg const& msg)"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.cancelPiece(msg);
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::InterestingUnchoking::cancelMsg(CancelMsg const& msg)"
                << std::endl;
        }

        context.setState(endState);
    }
    catch (...)
    {
        context.setState(endState);
        throw;
    }

    return;
}

void UploadMap_InterestingUnchoking::chokePeer(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::InterestingUnchoking"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::InterestingUnchoking::chokePeer()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getChokeMsg());
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::InterestingUnchoking::chokePeer()"
                << std::endl;
        }

        context.setState(UploadMap::InterestingChoking);
    }
    catch (...)
    {
        context.setState(UploadMap::InterestingChoking);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void UploadMap_InterestingUnchoking::notInterestedMsg(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::InterestingUnchoking"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::InterestingUnchoking::notInterestedMsg()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.setInterested(false);
        ctxt.callChokeAlgorithm();
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::InterestingUnchoking::notInterestedMsg()"
                << std::endl;
        }

        context.setState(UploadMap::NotInterestingUnchoking);
    }
    catch (...)
    {
        context.setState(UploadMap::NotInterestingUnchoking);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void UploadMap_InterestingUnchoking::requestMsg(UploadSMContext& context, RequestMsg const& msg)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::InterestingUnchoking"
            << std::endl;
    }

    UploadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::InterestingUnchoking::requestMsg(RequestMsg const& msg)"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.requestPieceMsg(msg);
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::InterestingUnchoking::requestMsg(RequestMsg const& msg)"
                << std::endl;
        }

        context.setState(endState);
    }
    catch (...)
    {
        context.setState(endState);
        throw;
    }

    return;
}

void UploadMap_InterestingUnchoking::sendPieceMsg(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::InterestingUnchoking"
            << std::endl;
    }

    UploadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::InterestingUnchoking::sendPieceMsg()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getPieceMsg());
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::InterestingUnchoking::sendPieceMsg()"
                << std::endl;
        }

        context.setState(endState);
    }
    catch (...)
    {
        context.setState(endState);
        throw;
    }

    return;
}

void UploadMap_InterestingUnchoking::stopMachine(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::InterestingUnchoking"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::InterestingUnchoking::stopMachine()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.callChokeAlgorithm();
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::InterestingUnchoking::stopMachine()"
                << std::endl;
        }

        context.setState(UploadMap::Stopped);
    }
    catch (...)
    {
        context.setState(UploadMap::Stopped);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void UploadMap_InterestingUnchoking::uploadRateTimer(UploadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : UploadMap::InterestingUnchoking"
            << std::endl;
    }

    UploadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: UploadMap::InterestingUnchoking::uploadRateTimer()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.renewUploadRateTimer();
        ctxt.calculateUploadRate();
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : UploadMap::InterestingUnchoking::uploadRateTimer()"
                << std::endl;
        }

        context.setState(endState);
    }
    catch (...)
    {
        context.setState(endState);
        throw;
    }

    return;
}

/*
 * Local variables:
 *  buffer-read-only: t
 * End:
 */

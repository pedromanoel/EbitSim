/*
 * ex: set ro:
 * DO NOT EDIT.
 * generated by smc (http://smc.sourceforge.net/)
 * from file : DownloadSM_sm.sm
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


#include "PeerWireThread.h"
#include "out/DownloadSM_sm.h"

using namespace statemap;

// Static class declarations.
DownloadMap_NotInterestedChoked DownloadMap::NotInterestedChoked("DownloadMap::NotInterestedChoked", 0);
DownloadMap_InterestedChoked DownloadMap::InterestedChoked("DownloadMap::InterestedChoked", 1);
DownloadMap_NotInterestedUnchoked DownloadMap::NotInterestedUnchoked("DownloadMap::NotInterestedUnchoked", 2);
DownloadMap_InterestedUnchoked DownloadMap::InterestedUnchoked("DownloadMap::InterestedUnchoked", 3);
DownloadMap_Stopped DownloadMap::Stopped("DownloadMap::Stopped", 4);

void DownloadSMState::bitFieldMsg(DownloadSMContext& context, BitFieldMsg const& msg)
{
    Default(context);
    return;
}

void DownloadSMState::chokeMsg(DownloadSMContext& context)
{
    Default(context);
    return;
}

void DownloadSMState::downloadRateTimer(DownloadSMContext& context)
{
    Default(context);
    return;
}

void DownloadSMState::haveMsg(DownloadSMContext& context, HaveMsg const& msg)
{
    Default(context);
    return;
}

void DownloadSMState::peerInteresting(DownloadSMContext& context)
{
    Default(context);
    return;
}

void DownloadSMState::peerNotInteresting(DownloadSMContext& context)
{
    Default(context);
    return;
}

void DownloadSMState::pieceMsg(DownloadSMContext& context, PieceMsg const& msg)
{
    Default(context);
    return;
}

void DownloadSMState::snubbedTimer(DownloadSMContext& context)
{
    Default(context);
    return;
}

void DownloadSMState::stopMachine(DownloadSMContext& context)
{
    Default(context);
    return;
}

void DownloadSMState::unchokeMsg(DownloadSMContext& context)
{
    Default(context);
    return;
}

void DownloadSMState::Default(DownloadSMContext& context)
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

void DownloadMap_Default::snubbedTimer(DownloadSMContext& context)
{

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::Default"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::Default::snubbedTimer()"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : DownloadMap::Default::snubbedTimer()"
            << std::endl;
    }


    return;
}

void DownloadMap_Default::downloadRateTimer(DownloadSMContext& context)
{

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::Default"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::Default::downloadRateTimer()"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : DownloadMap::Default::downloadRateTimer()"
            << std::endl;
    }


    return;
}

void DownloadMap_Default::stopMachine(DownloadSMContext& context)
{

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::Default"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::Default::stopMachine()"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : DownloadMap::Default::stopMachine()"
            << std::endl;
    }

    context.setState(DownloadMap::Stopped);
    (context.getState()).Entry(context);

    return;
}

void DownloadMap_Default::haveMsg(DownloadSMContext& context, HaveMsg const& msg)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::Default"
            << std::endl;
    }

    DownloadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::Default::haveMsg(HaveMsg const& msg)"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.processHaveMsg(msg);
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::Default::haveMsg(HaveMsg const& msg)"
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

void DownloadMap_NotInterestedChoked::Entry(DownloadSMContext& context)

{
    PeerWireThread& ctxt(context.getOwner());

    ctxt.printDebugMsgDownload("Entering state NotInterestedChoked");
    return;
}

void DownloadMap_NotInterestedChoked::bitFieldMsg(DownloadSMContext& context, BitFieldMsg const& msg)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::NotInterestedChoked"
            << std::endl;
    }

    DownloadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::NotInterestedChoked::bitFieldMsg(BitFieldMsg const& msg)"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.addBitField(msg);
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::NotInterestedChoked::bitFieldMsg(BitFieldMsg const& msg)"
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

void DownloadMap_NotInterestedChoked::chokeMsg(DownloadSMContext& context)
{

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::NotInterestedChoked"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::NotInterestedChoked::chokeMsg()"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : DownloadMap::NotInterestedChoked::chokeMsg()"
            << std::endl;
    }


    return;
}

void DownloadMap_NotInterestedChoked::peerInteresting(DownloadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::NotInterestedChoked"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::NotInterestedChoked::peerInteresting()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getInterestedMsg());
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::NotInterestedChoked::peerInteresting()"
                << std::endl;
        }

        context.setState(DownloadMap::InterestedChoked);
    }
    catch (...)
    {
        context.setState(DownloadMap::InterestedChoked);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void DownloadMap_NotInterestedChoked::unchokeMsg(DownloadSMContext& context)
{

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::NotInterestedChoked"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::NotInterestedChoked::unchokeMsg()"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : DownloadMap::NotInterestedChoked::unchokeMsg()"
            << std::endl;
    }

    context.setState(DownloadMap::NotInterestedUnchoked);
    (context.getState()).Entry(context);

    return;
}

void DownloadMap_InterestedChoked::Entry(DownloadSMContext& context)

{
    PeerWireThread& ctxt(context.getOwner());

    ctxt.printDebugMsgDownload("Entering state InterestedChoked");
    return;
}

void DownloadMap_InterestedChoked::chokeMsg(DownloadSMContext& context)
{

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::InterestedChoked"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::InterestedChoked::chokeMsg()"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : DownloadMap::InterestedChoked::chokeMsg()"
            << std::endl;
    }


    return;
}

void DownloadMap_InterestedChoked::peerNotInteresting(DownloadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::InterestedChoked"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::InterestedChoked::peerNotInteresting()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getNotInterestedMsg());
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::InterestedChoked::peerNotInteresting()"
                << std::endl;
        }

        context.setState(DownloadMap::NotInterestedChoked);
    }
    catch (...)
    {
        context.setState(DownloadMap::NotInterestedChoked);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void DownloadMap_InterestedChoked::pieceMsg(DownloadSMContext& context, PieceMsg const& msg)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::InterestedChoked"
            << std::endl;
    }

    DownloadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::InterestedChoked::pieceMsg(PieceMsg const& msg)"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.processBlock(msg);
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::InterestedChoked::pieceMsg(PieceMsg const& msg)"
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

void DownloadMap_InterestedChoked::unchokeMsg(DownloadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::InterestedChoked"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::InterestedChoked::unchokeMsg()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getRequestMsgBundle());
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::InterestedChoked::unchokeMsg()"
                << std::endl;
        }

        context.setState(DownloadMap::InterestedUnchoked);
    }
    catch (...)
    {
        context.setState(DownloadMap::InterestedUnchoked);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void DownloadMap_NotInterestedUnchoked::Entry(DownloadSMContext& context)

{
    PeerWireThread& ctxt(context.getOwner());

    ctxt.printDebugMsgDownload("Entering state NotInterestedUnchoked");
    return;
}

void DownloadMap_NotInterestedUnchoked::chokeMsg(DownloadSMContext& context)
{

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::NotInterestedUnchoked"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::NotInterestedUnchoked::chokeMsg()"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : DownloadMap::NotInterestedUnchoked::chokeMsg()"
            << std::endl;
    }

    context.setState(DownloadMap::NotInterestedChoked);
    (context.getState()).Entry(context);

    return;
}

void DownloadMap_NotInterestedUnchoked::peerInteresting(DownloadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::NotInterestedUnchoked"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::NotInterestedUnchoked::peerInteresting()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getInterestedMsg());
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getRequestMsgBundle());
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::NotInterestedUnchoked::peerInteresting()"
                << std::endl;
        }

        context.setState(DownloadMap::InterestedUnchoked);
    }
    catch (...)
    {
        context.setState(DownloadMap::InterestedUnchoked);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void DownloadMap_NotInterestedUnchoked::pieceMsg(DownloadSMContext& context, PieceMsg const& msg)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::NotInterestedUnchoked"
            << std::endl;
    }

    DownloadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::NotInterestedUnchoked::pieceMsg(PieceMsg const& msg)"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.processBlock(msg);
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::NotInterestedUnchoked::pieceMsg(PieceMsg const& msg)"
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

void DownloadMap_NotInterestedUnchoked::unchokeMsg(DownloadSMContext& context)
{

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::NotInterestedUnchoked"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::NotInterestedUnchoked::unchokeMsg()"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : DownloadMap::NotInterestedUnchoked::unchokeMsg()"
            << std::endl;
    }


    return;
}

void DownloadMap_InterestedUnchoked::Entry(DownloadSMContext& context)

{
    PeerWireThread& ctxt(context.getOwner());

    ctxt.printDebugMsgDownload("Entering state InterestedUnchoked");
    ctxt.startDownloadTimers();
    ctxt.setSnubbed(false);
    return;
}

void DownloadMap_InterestedUnchoked::Exit(DownloadSMContext& context)

{
    PeerWireThread& ctxt(context.getOwner());

    ctxt.stopDownloadTimers();
    ctxt.setSnubbed(false);
    return;
}

void DownloadMap_InterestedUnchoked::chokeMsg(DownloadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::InterestedUnchoked"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::InterestedUnchoked::chokeMsg()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.cancelDownloadRequests();
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::InterestedUnchoked::chokeMsg()"
                << std::endl;
        }

        context.setState(DownloadMap::InterestedChoked);
    }
    catch (...)
    {
        context.setState(DownloadMap::InterestedChoked);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void DownloadMap_InterestedUnchoked::downloadRateTimer(DownloadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::InterestedUnchoked"
            << std::endl;
    }

    DownloadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::InterestedUnchoked::downloadRateTimer()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.renewDownloadRateTimer();
        ctxt.calculateDownloadRate();
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::InterestedUnchoked::downloadRateTimer()"
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

void DownloadMap_InterestedUnchoked::peerNotInteresting(DownloadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::InterestedUnchoked"
            << std::endl;
    }

    (context.getState()).Exit(context);
    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::InterestedUnchoked::peerNotInteresting()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getNotInterestedMsg());
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::InterestedUnchoked::peerNotInteresting()"
                << std::endl;
        }

        context.setState(DownloadMap::NotInterestedUnchoked);
    }
    catch (...)
    {
        context.setState(DownloadMap::NotInterestedUnchoked);
        throw;
    }
    (context.getState()).Entry(context);

    return;
}

void DownloadMap_InterestedUnchoked::pieceMsg(DownloadSMContext& context, PieceMsg const& msg)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::InterestedUnchoked"
            << std::endl;
    }

    DownloadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::InterestedUnchoked::pieceMsg(PieceMsg const& msg)"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.renewSnubbedTimer();
        ctxt.processBlock(msg);
        ctxt.outgoingPeerWireMsg_ConnectionSM(ctxt.getRequestMsgBundle());
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::InterestedUnchoked::pieceMsg(PieceMsg const& msg)"
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

void DownloadMap_InterestedUnchoked::snubbedTimer(DownloadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::InterestedUnchoked"
            << std::endl;
    }

    DownloadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::InterestedUnchoked::snubbedTimer()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.setSnubbed(true);
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::InterestedUnchoked::snubbedTimer()"
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

void DownloadMap_InterestedUnchoked::unchokeMsg(DownloadSMContext& context)
{

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::InterestedUnchoked"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::InterestedUnchoked::unchokeMsg()"
            << std::endl;
    }

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "EXIT TRANSITION : DownloadMap::InterestedUnchoked::unchokeMsg()"
            << std::endl;
    }


    return;
}

void DownloadMap_Stopped::Entry(DownloadSMContext& context)

{
    PeerWireThread& ctxt(context.getOwner());

    ctxt.stopDownloadTimers();
    ctxt.printDebugMsgDownload("Entering state Closed");
    return;
}

void DownloadMap_Stopped::Default(DownloadSMContext& context)
{
    PeerWireThread& ctxt(context.getOwner());

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "LEAVING STATE   : DownloadMap::Stopped"
            << std::endl;
    }

    DownloadSMState& endState = context.getState();

    if (context.getDebugFlag() == true)
    {
        std::ostream& str = context.getDebugStream();

        str << "ENTER TRANSITION: DownloadMap::Stopped::Default()"
            << std::endl;
    }

    context.clearState();
    try
    {
        ctxt.printDebugMsgDownload("Thread terminated");
        if (context.getDebugFlag() == true)
        {
            std::ostream& str = context.getDebugStream();

            str << "EXIT TRANSITION : DownloadMap::Stopped::Default()"
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

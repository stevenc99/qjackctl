// qjackctlPatchbayRack.cpp
//
/****************************************************************************
   Copyright  (C) 2003, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or  ( at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#include "qjackctlPatchbayRack.h"

#include <qregexp.h>

#include <stdlib.h>


//----------------------------------------------------------------------
// class qjackctlPatchbaySocket -- Patchbay socket implementation.
//

// Constructor.
qjackctlPatchbaySocket::qjackctlPatchbaySocket ( const QString& sSocketName, const QString& sClientName, int iSocketType )
{
    m_sSocketName = sSocketName;
    m_sClientName = sClientName;
    m_iSocketType = iSocketType;
}

// Default destructor.
qjackctlPatchbaySocket::~qjackctlPatchbaySocket (void)
{
    m_pluglist.clear();
}


// Socket property accessors.
const QString& qjackctlPatchbaySocket::name (void)
{
    return m_sSocketName;
}

const QString& qjackctlPatchbaySocket::clientName (void)
{
    return m_sClientName;
}

int qjackctlPatchbaySocket::type (void)
{
    return m_iSocketType;
}

// Slot property methods.
void qjackctlPatchbaySocket::setName ( const QString& sSocketName )
{
    m_sSocketName = sSocketName;
}

void qjackctlPatchbaySocket::setClientName ( const QString& sClientName )
{
    m_sClientName = sClientName;
}

void qjackctlPatchbaySocket::setType ( int iSocketType )
{
    m_iSocketType = iSocketType;
}

// Plug list primitive methods.
void qjackctlPatchbaySocket::addPlug ( const QString& sPlugName )
{
    m_pluglist.append(sPlugName);
}

void qjackctlPatchbaySocket::removePlug ( const QString& sPlugName )
{
    QStringList::Iterator iter = m_pluglist.find(sPlugName);
    if (iter != m_pluglist.end())
        m_pluglist.remove(iter);
}


// Plug list accessor.
QStringList& qjackctlPatchbaySocket::pluglist (void)
{
    return m_pluglist;
}


//----------------------------------------------------------------------
// class qjackctlPatchbaySlot -- Patchbay socket slot implementation.
//

// Constructor.
qjackctlPatchbaySlot::qjackctlPatchbaySlot ( const QString& sSlotName, int iSlotMode )
{
    m_pOutputSocket = NULL;
    m_pInputSocket = NULL;

    m_sSlotName = sSlotName;
    m_iSlotMode = iSlotMode;
}

// Default destructor.
qjackctlPatchbaySlot::~qjackctlPatchbaySlot (void)
{
    setOutputSocket(NULL);
    setInputSocket(NULL);
}


// Slot property accessors.
const QString& qjackctlPatchbaySlot::name (void)
{
    return m_sSlotName;
}

int qjackctlPatchbaySlot::mode (void)
{
    return m_iSlotMode;
}


// Slot property methods.
void qjackctlPatchbaySlot::setName ( const QString& sSlotName )
{
    m_sSlotName = sSlotName;
}

void qjackctlPatchbaySlot::setMode ( int iSlotMode )
{
    m_iSlotMode = iSlotMode;
}


// Socket methods.
void qjackctlPatchbaySlot::setOutputSocket ( qjackctlPatchbaySocket *pSocket )
{
    m_pOutputSocket = pSocket;
}

void qjackctlPatchbaySlot::setInputSocket ( qjackctlPatchbaySocket *pSocket )
{
    m_pInputSocket = pSocket;
}


// Socket accessors.
qjackctlPatchbaySocket *qjackctlPatchbaySlot::outputSocket (void)
{
    return m_pOutputSocket;
}

qjackctlPatchbaySocket *qjackctlPatchbaySlot::inputSocket (void)
{
    return m_pInputSocket;
}


//----------------------------------------------------------------------
// class qjackctlPatchbayCable -- Patchbay cable connection implementation.
//

// Constructor.
qjackctlPatchbayCable::qjackctlPatchbayCable ( qjackctlPatchbaySocket *pOutputSocket, qjackctlPatchbaySocket *pInputSocket )
{
    m_pOutputSocket = pOutputSocket;
    m_pInputSocket = pInputSocket;
}

// Default destructor.
qjackctlPatchbayCable::~qjackctlPatchbayCable (void)
{
    setOutputSocket(NULL);
    setInputSocket(NULL);
}


// Socket methods.
void qjackctlPatchbayCable::setOutputSocket ( qjackctlPatchbaySocket *pSocket )
{
    m_pOutputSocket = pSocket;
}

void qjackctlPatchbayCable::setInputSocket ( qjackctlPatchbaySocket *pSocket )
{
    m_pInputSocket = pSocket;
}


// Socket accessors.
qjackctlPatchbaySocket *qjackctlPatchbayCable::outputSocket (void)
{
    return m_pOutputSocket;
}

qjackctlPatchbaySocket *qjackctlPatchbayCable::inputSocket (void)
{
    return m_pInputSocket;
}


//----------------------------------------------------------------------
// class qjackctlPatchbayRack -- Patchbay rack profile implementation.
//

// Constructor.
qjackctlPatchbayRack::qjackctlPatchbayRack (void)
{
    m_osocketlist.setAutoDelete(true);
    m_isocketlist.setAutoDelete(true);
    m_slotlist.setAutoDelete(true);
    m_cablelist.setAutoDelete(true);

    m_pJackClient = NULL;
    m_ppszOAudioPorts = NULL;
    m_ppszIAudioPorts = NULL;
    
    // MIDI connection persistence cache variables.
    m_omidiports.setAutoDelete(true);
    m_imidiports.setAutoDelete(true);
    m_pAlsaSeq = NULL;
}

// Default destructor.
qjackctlPatchbayRack::~qjackctlPatchbayRack (void)
{
    clear();
}


// Common socket list primitive methods.
void qjackctlPatchbayRack::addSocket ( QPtrList<qjackctlPatchbaySocket>& socketlist, qjackctlPatchbaySocket *pSocket )
{
    qjackctlPatchbaySocket *pSocketPtr = findSocket(socketlist, pSocket->name());
    if (pSocketPtr)
        socketlist.remove(pSocketPtr);

    socketlist.append(pSocket);
}


void qjackctlPatchbayRack::removeSocket ( QPtrList<qjackctlPatchbaySocket>& socketlist, qjackctlPatchbaySocket *pSocket )
{
    socketlist.remove(pSocket);
}


// Slot list primitive methods.
void qjackctlPatchbayRack::addSlot ( qjackctlPatchbaySlot *pSlot )
{
    qjackctlPatchbaySlot *pSlotPtr = findSlot(pSlot->name());
    if (pSlotPtr)
        m_slotlist.remove(pSlotPtr);

    m_slotlist.append(pSlot);
}

void qjackctlPatchbayRack::removeSlot ( qjackctlPatchbaySlot *pSlot )
{
    m_slotlist.remove(pSlot);
}


// Cable list primitive methods.
void qjackctlPatchbayRack::addCable ( qjackctlPatchbayCable *pCable )
{
    qjackctlPatchbayCable *pCablePtr = findCable(pCable);
    if (pCablePtr)
        m_cablelist.remove(pCablePtr);

    m_cablelist.append(pCable);
}

void qjackctlPatchbayRack::removeCable ( qjackctlPatchbayCable *pCable )
{
    m_cablelist.remove(pCable);
}


// Common socket finders.
qjackctlPatchbaySocket *qjackctlPatchbayRack::findSocket ( QPtrList<qjackctlPatchbaySocket>& socketlist, const QString& sSocketName )
{
    for (qjackctlPatchbaySocket *pSocket = socketlist.first(); pSocket; pSocket = socketlist.next()) {
        if (sSocketName == pSocket->name())
            return pSocket;
    }

    return NULL;
}


// Patchbay socket slot finders.
qjackctlPatchbaySlot *qjackctlPatchbayRack::findSlot ( const QString& sSlotName )
{
    for (qjackctlPatchbaySlot *pSlot = m_slotlist.first(); pSlot; pSlot = m_slotlist.next()) {
        if (sSlotName == pSlot->name())
            return pSlot;
    }

    return NULL;
}


// Cable finder.
qjackctlPatchbayCable *qjackctlPatchbayRack::findCable ( const QString& sOutputSocket, const QString& sInputSocket )
{
    qjackctlPatchbaySocket *pSocket;

    for (qjackctlPatchbayCable *pCable = m_cablelist.first(); pCable; pCable = m_cablelist.next()) {
        pSocket = pCable->outputSocket();
        if (pSocket && sOutputSocket == pSocket->name()) {
            pSocket = pCable->inputSocket();
            if (pSocket && sInputSocket == pSocket->name())
                return pCable;
        }
    }

    return NULL;
}

qjackctlPatchbayCable *qjackctlPatchbayRack::findCable ( qjackctlPatchbayCable *pCablePtr )
{
    QString sOutputSocket;
    if (pCablePtr->outputSocket())
        sOutputSocket = pCablePtr->outputSocket()->name();

    QString sInputSocket;
    if (pCablePtr->inputSocket())
        sInputSocket = pCablePtr->inputSocket()->name();

    return findCable(sOutputSocket, sInputSocket);
}


// Patckbay cleaners.
void qjackctlPatchbayRack::clear (void)
{
    m_cablelist.clear();
    m_slotlist.clear();
    m_isocketlist.clear();
    m_osocketlist.clear();
}


// Patchbay rack output sockets list accessor.
QPtrList<qjackctlPatchbaySocket>& qjackctlPatchbayRack::osocketlist (void)
{
    return m_osocketlist;
}


// Patchbay rack input sockets list accessor.
QPtrList<qjackctlPatchbaySocket>& qjackctlPatchbayRack::isocketlist (void)
{
    return m_isocketlist;
}


// Patchbay rack slots list accessor.
QPtrList<qjackctlPatchbaySlot>& qjackctlPatchbayRack::slotlist (void)
{
    return m_slotlist;
}


// Patchbay cable connections list accessor.
QPtrList<qjackctlPatchbayCable>& qjackctlPatchbayRack::cablelist (void)
{
    return m_cablelist;
}


// Lookup for the n-th client port that matches the given regular expression...
const char *qjackctlPatchbayRack::findAudioPort ( const char **ppszAudioPorts, const QString& sClientName, const QString& sPortName, int n )
{
    QRegExp rxClientName(sClientName);
    QRegExp rxPortName(sPortName);

    int i = 0;
    int iClientPort = 0;
    while (ppszAudioPorts[iClientPort]) {
        QString sClientPort = ppszAudioPorts[iClientPort];
        int iColon = sClientPort.find(":");
        if (iColon >= 0) {
            if (rxClientName.exactMatch(sClientPort.left(iColon)) &&
                rxPortName.exactMatch(sClientPort.right(sClientPort.length() - iColon - 1))) {
                if (++i > n)
                    return ppszAudioPorts[iClientPort];
            }
        }
        iClientPort++;
    }

    return NULL;
}


// Check if an output client:port is already connected to yet another one.
bool qjackctlPatchbayRack::isAudioConnected ( const char *pszOutputPort, const char *pszInputPort )
{
    bool bConnected = false;

    const char **ppszInputPorts = jack_port_get_all_connections(m_pJackClient, jack_port_by_name(m_pJackClient, pszOutputPort));
    if (ppszInputPorts) {
        for (int i = 0 ; ppszInputPorts[i] && !bConnected; i++)
            bConnected = (strcmp(ppszInputPorts[i], pszInputPort) == 0);
    }
    ::free(ppszInputPorts);

    return bConnected;
}


// Audio port-pair connection executive.
bool qjackctlPatchbayRack::connectAudioPorts ( const char *pszOutputPort, const char *pszInputPort )
{
    return (jack_connect(m_pJackClient, pszOutputPort, pszInputPort) == 0);
}


// Check and maint whether an audio socket pair is fully connected.
void qjackctlPatchbayRack::connectAudioCable ( qjackctlPatchbaySocket *pOutputSocket, qjackctlPatchbaySocket *pInputSocket )
{
    if (pOutputSocket == NULL || pInputSocket == NULL)
        return;
    if (pOutputSocket->type() != pInputSocket->type() || pOutputSocket->type() != QJACKCTL_SOCKETTYPE_AUDIO)
        return;

    // Iterate on each corresponding plug...
    QStringList::Iterator iterOutputPlug = pOutputSocket->pluglist().begin();
    QStringList::Iterator iterInputPlug  = pInputSocket->pluglist().begin();
    while (iterOutputPlug != pOutputSocket->pluglist().end() &&
           iterInputPlug  != pInputSocket->pluglist().end()) {
        // Check audio port connection sequentially...
        int iPort = 0;
        const char *pszOutputPort;
        while ((pszOutputPort = findAudioPort(m_ppszOAudioPorts, pOutputSocket->clientName(), *iterOutputPlug, iPort)) != NULL) {
            const char *pszInputPort = findAudioPort(m_ppszIAudioPorts, pInputSocket->clientName(), *iterInputPlug, iPort);
            if (pszInputPort) {
                unsigned int uiCableFlags = QJACKCTL_CABLE_FAILED;
                if (isAudioConnected(pszOutputPort, pszInputPort))
                    uiCableFlags = QJACKCTL_CABLE_CHECKED;
                else if (connectAudioPorts(pszOutputPort, pszInputPort))
                    uiCableFlags = QJACKCTL_CABLE_CONNECTED;
                emit cableConnected(pszOutputPort, pszInputPort, uiCableFlags);
            }
            iPort++;
        }
        // Get on next plug pair...
        iterOutputPlug++;
        iterInputPlug++;
    }
}


// Main audio cable connect persistance scan cycle.
void qjackctlPatchbayRack::connectAudioScan ( jack_client_t *pJackClient )
{
    if (pJackClient == NULL || m_pJackClient)
        return;

    // Cache client descriptor.
    m_pJackClient = pJackClient;
    // Cache all current output client-ports...
    m_ppszOAudioPorts = jack_get_ports(m_pJackClient, 0, 0, JackPortIsOutput);
    if (m_ppszOAudioPorts == NULL)
        return;

    // Cache all current input client-ports...
    m_ppszIAudioPorts = jack_get_ports(m_pJackClient, 0, 0, JackPortIsInput);
    if (m_ppszIAudioPorts) {
        for (qjackctlPatchbayCable *pCable = m_cablelist.first(); pCable; pCable = m_cablelist.next())
            connectAudioCable(pCable->outputSocket(), pCable->inputSocket());
    }

    // Free client-ports caches...
    if (m_ppszOAudioPorts)
        ::free(m_ppszOAudioPorts);
    if (m_ppszIAudioPorts)
        ::free(m_ppszIAudioPorts);

    // Reset cached pointers.
    m_ppszOAudioPorts = NULL;
    m_ppszIAudioPorts  = NULL;
    m_pJackClient = NULL;
}


// Load all midi available midi ports of a given type.
void qjackctlPatchbayRack::loadMidiPorts ( QPtrList<qjackctlMidiPort>& midiports, bool bReadable )
{
    if (m_pAlsaSeq == NULL)
        return;

    midiports.clear();

    unsigned int uiAlsaFlags;
    if (bReadable)
        uiAlsaFlags = SND_SEQ_PORT_CAP_READ  | SND_SEQ_PORT_CAP_SUBS_READ;
    else
        uiAlsaFlags = SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE;

    snd_seq_client_info_t *pClientInfo;
    snd_seq_port_info_t   *pPortInfo;

    snd_seq_client_info_alloca(&pClientInfo);
    snd_seq_port_info_alloca(&pPortInfo);
    snd_seq_client_info_set_client(pClientInfo, -1);

    while (snd_seq_query_next_client(m_pAlsaSeq, pClientInfo) >= 0) {
        int iAlsaClient = snd_seq_client_info_get_client(pClientInfo);
        if (iAlsaClient > 0) {
            QString sClientName = snd_seq_client_info_get_name(pClientInfo);
            snd_seq_port_info_set_client(pPortInfo, iAlsaClient);
            snd_seq_port_info_set_port(pPortInfo, -1);
            while (snd_seq_query_next_port(m_pAlsaSeq, pPortInfo) >= 0) {
                unsigned int uiPortCapability = snd_seq_port_info_get_capability(pPortInfo);
                if (((uiPortCapability & uiAlsaFlags) == uiAlsaFlags) &&
                    ((uiPortCapability & SND_SEQ_PORT_CAP_NO_EXPORT) == 0)) {
                    qjackctlMidiPort *pMidiPort = new qjackctlMidiPort;
                    pMidiPort->sClientName = sClientName;
                    pMidiPort->iAlsaClient = iAlsaClient;
                    pMidiPort->sPortName   = snd_seq_port_info_get_name(pPortInfo);
                    pMidiPort->iAlsaPort   = snd_seq_port_info_get_port(pPortInfo);
                    midiports.append(pMidiPort);
                }
            }
        }
    }
}


// Lookup for the n-th MIDI client port that matches the given regular expression...
qjackctlMidiPort *qjackctlPatchbayRack::findMidiPort ( QPtrList<qjackctlMidiPort>& midiports, const QString& sClientName, const QString& sPortName, int n )
{
    QRegExp rxClientName(sClientName);
    QRegExp rxPortName(sPortName);

    int i = 0;
    // For each port...
    for (qjackctlMidiPort *pMidiPort = midiports.first(); pMidiPort; pMidiPort = midiports.next()) {
        if (rxClientName.exactMatch(pMidiPort->sClientName) &&
            rxPortName.exactMatch(pMidiPort->sPortName)) {
            if (++i > n)
                return pMidiPort;
        }
    }
    return NULL;
}


// Check if an MIDI output client:port is already connected to yet another one.
bool qjackctlPatchbayRack::isMidiConnected ( qjackctlMidiPort *pOutputPort, qjackctlMidiPort *pInputPort )
{
    bool bConnected = false;

    snd_seq_query_subscribe_t *pAlsaSubs;
    snd_seq_addr_t seq_addr;

    snd_seq_query_subscribe_alloca(&pAlsaSubs);

    // Get port connections...
    snd_seq_query_subscribe_set_type(pAlsaSubs, SND_SEQ_QUERY_SUBS_READ);
    snd_seq_query_subscribe_set_index(pAlsaSubs, 0);
    seq_addr.client = pOutputPort->iAlsaClient;
    seq_addr.port   = pOutputPort->iAlsaPort;
    snd_seq_query_subscribe_set_root(pAlsaSubs, &seq_addr);

    while (!bConnected && snd_seq_query_port_subscribers(m_pAlsaSeq, pAlsaSubs) >= 0) {
        seq_addr = *snd_seq_query_subscribe_get_addr(pAlsaSubs);
        bConnected = (seq_addr.client == pInputPort->iAlsaClient && seq_addr.port == pInputPort->iAlsaPort);
        snd_seq_query_subscribe_set_index(pAlsaSubs, snd_seq_query_subscribe_get_index(pAlsaSubs) + 1);
    }

    return bConnected;
}


// MIDI port-pair connection executive.
bool qjackctlPatchbayRack::connectMidiPorts ( qjackctlMidiPort *pOutputPort, qjackctlMidiPort *pInputPort )
{
    snd_seq_port_subscribe_t *pAlsaSubs;
    snd_seq_addr_t seq_addr;

    snd_seq_port_subscribe_alloca(&pAlsaSubs);

    seq_addr.client = pOutputPort->iAlsaClient;
    seq_addr.port   = pOutputPort->iAlsaPort;
    snd_seq_port_subscribe_set_sender(pAlsaSubs, &seq_addr);

    seq_addr.client = pInputPort->iAlsaClient;
    seq_addr.port   = pInputPort->iAlsaPort;
    snd_seq_port_subscribe_set_dest(pAlsaSubs, &seq_addr);

    return (snd_seq_subscribe_port(m_pAlsaSeq, pAlsaSubs) == 0);
}


// Check and maint whether a MIDI socket pair is fully connected.
void qjackctlPatchbayRack::connectMidiCable ( qjackctlPatchbaySocket *pOutputSocket, qjackctlPatchbaySocket *pInputSocket )
{
    if (pOutputSocket == NULL || pInputSocket == NULL)
        return;
    if (pOutputSocket->type() != pInputSocket->type() || pOutputSocket->type() != QJACKCTL_SOCKETTYPE_MIDI)
        return;

    // Iterate on each corresponding plug...
    QStringList::Iterator iterOutputPlug = pOutputSocket->pluglist().begin();
    QStringList::Iterator iterInputPlug  = pInputSocket->pluglist().begin();
    while (iterOutputPlug != pOutputSocket->pluglist().end() &&
           iterInputPlug  != pInputSocket->pluglist().end()) {
        // Check MIDI port connection sequentially...
        int iPort = 0;
        qjackctlMidiPort *pOutputPort;
        while ((pOutputPort = findMidiPort(m_omidiports, pOutputSocket->clientName(), *iterOutputPlug, iPort)) != NULL) {
            QString sOutputPort = QString::number(pOutputPort->iAlsaClient) + ":" + QString::number(pOutputPort->iAlsaPort) + " " + pOutputPort->sClientName;
            qjackctlMidiPort *pInputPort = findMidiPort(m_imidiports, pInputSocket->clientName(), *iterInputPlug, iPort);
            if (pInputPort) {
                QString sInputPort = QString::number(pInputPort->iAlsaClient) + ":" + QString::number(pInputPort->iAlsaPort) + " " + pInputPort->sClientName;
                unsigned int uiCableFlags = QJACKCTL_CABLE_FAILED;
                if (isMidiConnected(pOutputPort, pInputPort))
                    uiCableFlags = QJACKCTL_CABLE_CHECKED;
                else if (connectMidiPorts(pOutputPort, pInputPort))
                    uiCableFlags = QJACKCTL_CABLE_CONNECTED;
                emit cableConnected(sOutputPort, sInputPort, uiCableFlags);
            }
            iPort++;
        }
        // Get on next plug pair...
        iterOutputPlug++;
        iterInputPlug++;
    }
}


// Overloaded MIDI cable connect persistance scan cycle.
void qjackctlPatchbayRack::connectMidiScan ( snd_seq_t *pAlsaSeq )
{
    if (pAlsaSeq == NULL || m_pAlsaSeq)
        return;

    // Cache sequencer descriptor.
    m_pAlsaSeq = pAlsaSeq;
    // Cache all current output client-ports...
    loadMidiPorts(m_omidiports, true);
    loadMidiPorts(m_imidiports, false);

    // Run the MIDI cable scan...
    for (qjackctlPatchbayCable *pCable = m_cablelist.first(); pCable; pCable = m_cablelist.next())
        connectMidiCable(pCable->outputSocket(), pCable->inputSocket());

    // Free client-ports caches...
    m_omidiports.clear();
    m_imidiports.clear();
    m_pAlsaSeq = NULL;
}


// qjackctlPatchbayRack.cpp
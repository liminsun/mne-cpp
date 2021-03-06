//=============================================================================================================
/**
* @file     fiffsimulator.h
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     February, 2013
*
* @section  LICENSE
*
* Copyright (C) 2013, Christoph Dinh and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    Contains the declaration of the FiffSimulator class.
*
*/

#ifndef FIFFSIMULATOR_H
#define FIFFSIMULATOR_H


//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "fiffsimulator_global.h"

#include <scShared/Interfaces/ISensor.h>
#include <generics/circularbuffer_old.h>
#include <generics/circularmatrixbuffer.h>
#include <scMeas/newrealtimemultisamplearray.h>


//*************************************************************************************************************
//=============================================================================================================
// FIFF INCLUDES
//=============================================================================================================

#include <fiff/fiff_info.h>


//*************************************************************************************************************
//=============================================================================================================
// MNE INCLUDES
//=============================================================================================================

#include <rtClient/rtcmdclient.h>


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QtWidgets>
#include <QVector>
#include <QTimer>


//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE FiffSimulatorPlugin
//=============================================================================================================

namespace FiffSimulatorPlugin
{


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace SCSHAREDLIB;
using namespace IOBuffer;
using namespace RTCLIENTLIB;
using namespace FIFFLIB;
using namespace SCMEASLIB;


//*************************************************************************************************************
//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

class FiffSimulatorProducer;


//=============================================================================================================
/**
* DECLARE CLASS FiffSimulator
*
* @brief The FiffSimulator class provides a RT server connection.
*/
class FIFFSIMULATORSHARED_EXPORT FiffSimulator : public ISensor
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "scsharedlib/1.0" FILE "fiffsimulator.json") //NEw Qt5 Plugin system replaces Q_EXPORT_PLUGIN2 macro
    // Use the Q_INTERFACES() macro to tell Qt's meta-object system about the interfaces
    Q_INTERFACES(SCSHAREDLIB::ISensor)

    friend class FiffSimulatorProducer;
    friend class FiffSimulatorSetupWidget;

public:

    //=========================================================================================================
    /**
    * Constructs a FiffSimulator.
    */
    FiffSimulator();

    //=========================================================================================================
    /**
    * Destroys the FiffSimulator.
    */
    virtual ~FiffSimulator();

    //=========================================================================================================
    /**
    * Clears the rt server
    */
    void clear();

    //=========================================================================================================
    /**
    * Clone the plugin
    */
    virtual QSharedPointer<IPlugin> clone() const;

    //=========================================================================================================
    /**
    * Initialise the FiffSimulator.
    */
    virtual void init();

    //=========================================================================================================
    /**
    * Is called when plugin is detached of the stage. Can be used to safe settings.
    */
    virtual void unload();

    virtual bool start();
    virtual bool stop();

    virtual IPlugin::PluginType getType() const;
    virtual QString getName() const;

    virtual QWidget* setupWidget();

//slots:
    //=========================================================================================================
    /**
    * Change connector
    *
    * @param[in] p_iNewConnectorId      new connector ID
    */
    void changeConnector(qint32 p_iNewConnectorId);

    //=========================================================================================================
    /**
    * Connects the cmd client.
    */
    void connectCmdClient();

    //=========================================================================================================
    /**
    * Disconnects the cmd client.
    */
    void disconnectCmdClient();

    //=========================================================================================================
    /**
    * Request FiffInfo using cmd client and producer (data client)
    */
    void requestInfo();

signals:
    //=========================================================================================================
    /**
    * Emitted when command clients connection status changed
    *
    * @param[in] p_bStatus  connection status
    */
    void cmdConnectionChanged(bool p_bStatus);

    //=========================================================================================================
    /**
    * Emitted when fiffInfo is available
    */
    void fiffInfoAvailable();

protected:
    virtual void run();

private:
    //=========================================================================================================
    /**
    * Initialises the output connector.
    */
    void initConnector();


    QMutex m_qMutex;


    QString m_sFiffSimulatorClientAlias;     /**< The rt server client alias.*/

//    float           m_fSamplingRate;                /**< The sampling rate.*/
//    int             m_iDownsamplingFactor;          /**< The down sampling factor.*/

    PluginOutputData<NewRealTimeMultiSampleArray>::SPtr m_pRTMSA_FiffSimulator;     /**< The NewRealTimeMultiSampleArray to provide the rt_server Channels.*/

    QSharedPointer<RtCmdClient>                         m_pRtCmdClient;             /**< The command client.*/
    bool                                                m_bCmdClientIsConnected;    /**< If the command client is connected.*/

    QString                                             m_sFiffSimulatorIP;         /**< The IP Adress of mne_rt_server.*/
    QSharedPointer<FiffSimulatorProducer>               m_pFiffSimulatorProducer;   /**< Holds the FiffSimulatorProducer.*/


    QMap<qint32, QString>   m_qMapConnectors;               /**< Connector map.*/
    qint32                  m_iActiveConnectorId;           /**< The active connector.*/


    FiffInfo::SPtr  m_pFiffInfo;                            /**< Fiff measurement info.*/
    qint32          m_iBufferSize;                          /**< The raw data buffer size.*/

    QTimer          m_cmdConnectionTimer;                   /**< Timer for convinient command client connection. When timer times out a connection is tried to be established. */

    QSharedPointer<RawMatrixBuffer> m_pRawMatrixBuffer_In;  /**< Holds incoming raw data. */

    bool                            m_bIsRunning;           /**< Whether FiffSimulator is running.*/

};

} // NAMESPACE

#endif // FIFFSIMULATOR_H

//=============================================================================================================
/**
* @file     ssvepbci.h
* @author   Viktor Klüber <viktor.klueber@tu-ilmenau.de>;
*           Lorenz Esch <lorenz.esch@tu-ilmenau.de>;
*           Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>;
* @version  1.0
* @date     May, 2016
*
* @section  LICENSE
*
* Copyright (C) 2013, Lorenz Esch, Christoph Dinh and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of the Massachusetts General Hospital nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MASSACHUSETTS GENERAL HOSPITAL BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    Contains the declaration of the ssvepBCI class.
*
*/

#ifndef SSVEPBCI_H
#define SSVEPBCI_H


//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================
#include "ssvepbci_global.h"

#include <mne_x/Interfaces/IAlgorithm.h>

#include <generics/circularmatrixbuffer.h>

#include <xMeas/newrealtimesamplearray.h>
#include <xMeas/newrealtimemultisamplearray.h>
#include <xMeas/realtimesourceestimate.h>

#include <utils/FilterTools/filterdata.h>

#include <fstream>

//*************************************************************************************************************
//=============================================================================================================
// QT STL INCLUDES
//=============================================================================================================

#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>

#include "FormFiles/ssvepbcisetupwidget.h"
#include "FormFiles/ssvepbcisetupstimuluswidget.h"

//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE TMSIPlugin
//=============================================================================================================

namespace ssvepBCIPlugin
{

//*************************************************************************************************************
//=============================================================================================================
// TypeDefs
//=============================================================================================================

typedef QList< QList<double> > MyQList;

//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace MNEX;
using namespace XMEASLIB;
using namespace IOBuffer;
using namespace UTILSLIB;
using namespace std;


//=============================================================================================================
/**
* BCI...
*
* @brief The BCI class provides an EEG BCI.
*/
class SSVEPBCISHARED_EXPORT ssvepBCI : public IAlgorithm
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "mne_x/1.0" FILE "ssvepbci.json") //NEw Qt5 Plugin system replaces Q_EXPORT_PLUGIN2 macro
    // Use the Q_INTERFACES() macro to tell Qt's meta-object system about the interfaces
    Q_INTERFACES(MNEX::IAlgorithm)

    friend class BCISetupWidget;
    friend class BCIFeatureWindow;
    friend class ssvepBCISetupStimulusWidget;
    friend class ssvepBCISetupWidget;

public:
    //=========================================================================================================
    /**
    * Constructs a BCI.
    */
    ssvepBCI();

    //=========================================================================================================
    /**
    * Destroys the BCI.
    */
    virtual ~ssvepBCI();

    //=========================================================================================================
    /**
    * Clone the plugin
    */
    virtual QSharedPointer<IPlugin> clone() const;

    //=========================================================================================================
    /**
    * Initialise input and output connectors.
    */
    virtual void init();

    //=========================================================================================================
    /**
    * Starts data recording
    */
    void showSetupStimulus();

    //=========================================================================================================
    /**
    * Is called when plugin is detached of the stage. Can be used to safe settings.
    */
    virtual void unload();

    //=========================================================================================================
    /**
    * Starts the BCI by starting the BCI's thread.
    */
    virtual bool start();

    //=========================================================================================================
    /**
    * Stops the BCI by stopping the BCI's thread.
    */
    virtual bool stop();

    virtual IPlugin::PluginType getType() const;
    virtual QString getName() const;

    virtual QWidget* setupWidget();


protected:
    /**
    * This update function gets called whenever the input buffer stream from the TMSI plugin is full and need to be emptied by this BCI plugin.
    *
    * @param [in] pMeasurement measurement object.
    */
    void updateSensor(XMEASLIB::NewMeasurement::SPtr pMeasurement);

    //=========================================================================================================
    /**
    * This update function gets called whenever the input buffer stream from the Sourcelab plugin is full and need to be emptied by this BCI plugin.
    *
    * @param [in] pMeasurement measurement object.
    */
    void updateSource(XMEASLIB::NewMeasurement::SPtr pMeasurement);


    //=========================================================================================================
    /**
    * Clears features
    *
    */
    void clearFeatures();

    //=========================================================================================================
    /**
    * Clears all classification results
    *
    */
    void clearClassifications();


    //=========================================================================================================
    /**
    * Look for trigger in stim channel
    *
    */
    bool lookForTrigger(const MatrixXd &data);

    //=========================================================================================================
    /**
    * reading from sliding time window and write it to the data Matrix
    *
    */
    void readFromSlidingTimeWindow(MatrixXd &data);

    //=========================================================================================================
    /**
    * The starting point for the thread. After calling start(), the newly created thread calls this function.
    * Returning from this method will end the execution of the thread.
    * Pure virtual method inherited by QThread.
    */
    virtual void run();

    //=========================================================================================================
    /**
    * Do BCI stuff with data received from sensor level
    *
    */
    void BCIOnSensorLevel();


signals:
    void paintFeatures(MyQList features, bool bTrigerActivated);

private:
    QAction*                                            m_pActionSetupStimulus;             /**< starts stimulus feature */

    QSharedPointer<ssvepBCISetupStimulusWidget>         m_pssvepBCISetupStimulusWidget; /**< Widget for stimulus setup */

    PluginInputData<NewRealTimeMultiSampleArray>::SPtr  m_pRTMSAInput;          /**< The RealTimeMultiSampleArray input.*/
    PluginInputData<RealTimeSourceEstimate>::SPtr       m_pRTSEInput;           /**< The RealTimeSourceEstimate input.*/

    // sliding Window
    int                     m_iCounter;                         /**< iterative index for counting the time segments in the time window */
    int                     m_iSampleFrequency;                 /**< sample frequency of the device [Hz] */
    int                     m_iReadSampleSize;             /**< numbers of sample for one time segment (about 0.1 seconds) */
    int                     m_iWriteSampleSize;            /**< numbers of sample for writing to the time window  */
    int                     m_iTimeWindowSegmentSize;     /**< needed size of the buffer for reading with an adaptable sliding window */
    int                     m_iWriteIndex;   /**< Index for writing a new increment from the buffer to the time window */
    int                     m_iReadIndex;                       /**< index for reading from the time window */
    int                     m_iReadToWriteBuffer;               /**< number of samples from the current readindex to current write index */
    int                     m_iWindowSize;
//    QFile                   m_qFile;
//    QTextStream             m_sOut;
    //int                     m_iThrottle;                        /**< increment for throtteling sample rate */


    // Sensor level
    FiffInfo::SPtr          m_pFiffInfo_Sensor;                 /**< Sensor level: Fiff information for sensor data. */
    bool                    m_bFiffInfoInitialised_Sensor;      /**< Sensor level: Fiff information initialised. */
    bool                    m_bFillSensorWindowFirstTime;       /**< Sensor level: Flag if the working matrix m_mSlidingWindowSensor is being filled for the first time. */
    MatrixXd                m_matSlidingWindowSensor;           /**< Sensor level: Working (sliding) matrix, used to store data for feature calculation on sensor level. */
    MatrixXd                m_matTimeBetweenWindowsSensor;      /**< Sensor level: Samples stored during time between windows on sensor level. */
    int                     m_iTBWIndexSensor;                  /**< Sensor level: Index of the amount of data which was already filled during the time between windows. */
    int                     m_iNumberOfCalculatedFeatures;      /**< Sensor level: Index which is iterated until enough features are calculated and classified to generate a final classifcation result.*/
    QVector< VectorXd >     m_vLoadedSensorBoundary;            /**< Sensor level: Loaded decision boundary on sensor level. */
    QStringList             m_slChosenFeatureSensor;            /**< Sensor level: Features used to calculate data points in feature space on sensor level. */
    QMap<QString, int>      m_mapElectrodePinningScheme;        /**< Sensor level: Loaded pinning scheme of the Duke 128 EEG cap. */
    QList< QPair< int,QList<double> > >  m_lFeaturesSensor;     /**< Sensor level: Features calculated on sensor level. */
    QList<double>           m_lClassResultsSensor;              /**< Sensor level: Classification results on sensor level. */
    MatrixXd                m_matStimChannelSensor;             /**< Sensor level: Stim channel. */
    MatrixXd                m_matTimeBetweenWindowsStimSensor;  /**< Sensor level: Stim channel. */
    QList<int>              m_lElectrodeNumbers;                /**< Sensor level: number of chosen electrodes */

    MatrixXd                m_matSlidingTimeWindow;             /**< Sensor Level: adaptational sliding time window. */



    // old privates
    PluginOutputData<NewRealTimeSampleArray>::SPtr      m_pBCIOutputOne;        /**< The first RealTimeSampleArray of the BCI output.*/
    PluginOutputData<NewRealTimeSampleArray>::SPtr      m_pBCIOutputTwo;        /**< The second RealTimeSampleArray of the BCI output.*/
    PluginOutputData<NewRealTimeSampleArray>::SPtr      m_pBCIOutputThree;      /**< The third RealTimeSampleArray of the BCI output.*/
    PluginOutputData<NewRealTimeSampleArray>::SPtr      m_pBCIOutputFour;       /**< The fourth RealTimeSampleArray of the BCI output.*/
    PluginOutputData<NewRealTimeSampleArray>::SPtr      m_pBCIOutputFive;       /**< The fifth RealTimeSampleArray of the BCI output.*/


    CircularMatrixBuffer<double>::SPtr                  m_pBCIBuffer_Sensor;    /**< Holds incoming sensor level data.*/
    CircularMatrixBuffer<double>::SPtr                  m_pBCIBuffer_Source;    /**< Holds incoming source level data.*/

    QSharedPointer<FilterData>                          m_filterOperator;       /**< Holds filter with specified properties by the user.*/

//    QSharedPointer<BCIFeatureWindow>                    m_BCIFeatureWindow;     /**< Holds pointer to BCIFeatureWindow for visualization purposes.*/

    ofstream                m_outStreamDebug;                   /**< Outputstream to generate debug file.*/

    bool                    m_bIsRunning;                       /**< Whether BCI is running.*/
    QString                 m_qStringResourcePath;              /**< The path to the BCI resource directory.*/
    bool                    m_bProcessData;                     /**< Whether BCI is to get data out of the continous input data stream, i.e. the EEG data from sensor level.*/
    bool                    m_bTriggerActivated;                /**< Whether the trigger was activated.*/
    QMutex                  m_qMutex;                           /**< QMutex to guarantee thread safety.*/


    // Source level
    QVector< VectorXd >     m_vLoadedSourceBoundary;            /**< Source level: Loaded decision boundary on source level. */
    QStringList             m_slChosenFeatureSource;            /**< Source level: Features used to calculate data points in feature space on source level. */
    QMap<QString, int>      m_mapDestrieuxAtlasRegions;         /**< Source level: Loaded Destrieux atlas regions. */

    // GUI variables
    bool                    m_bUseSensorData;                   /**< GUI input: Use sensor data stream. */

};

} // NAMESPACE

#endif // SSVEPBCI_H

//=============================================================================================================
/**
* @file     babymeg.cpp
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Limin Sun <liminsun@nmr.mgh.harvard.edu>;
*           Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     February, 2013
*
* @section  LICENSE
*
* Copyright (C) 2013, Christoph Dinh, Limin Sun, Lorenz Esch and Matti Hamalainen. All rights reserved.
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
* @brief    BabyMEG class definition.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "babymeg.h"
#include "FormFiles/babymegsetupwidget.h"
#include "FormFiles/babymegprojectdialog.h"
#include "FormFiles/babymegsquidcontroldgl.h"
#include "FormFiles/babymeghpidgl.h"
#include "babymegclient.h"
#include "babymeginfo.h"

#include <iostream>

#include <utils/ioutils.h>
#include <utils/detecttrigger.h>
#include <fiff/fiff_types.h>
#include <fiff/fiff_dir_tree.h>
#include <rtClient/rtcmdclient.h>
#include <scMeas/newrealtimemultisamplearray.h>


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QtCore/QtPlugin>
#include <QtCore/QTextStream>
#include <QtCore/QFile>

#include <QList>
#include <QDebug>
#include <QDir>
#include <QDateTime>


//*************************************************************************************************************
//=============================================================================================================
// Eigen INCLUDES
//=============================================================================================================


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace BABYMEGPLUGIN;
using namespace UTILSLIB;
using namespace SCSHAREDLIB;
using namespace IOBuffer;
using namespace SCMEASLIB;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

BabyMEG::BabyMEG()
: m_iBlinkStatus(0)
, m_iBufferSize(-1)
, m_bWriteToFile(false)
, m_sCurrentParadigm("")
, m_bIsRunning(false)
, m_bUseRecordTimer(false)
, m_pRawMatrixBuffer(0)
, m_sFiffProjections(QCoreApplication::applicationDirPath() + "/mne_scan_plugins/resources/babymeg/header.fif")
, m_sFiffCompensators(QCoreApplication::applicationDirPath() + "/mne_scan_plugins/resources/babymeg/compensator.fif")
, m_sBadChannels(QCoreApplication::applicationDirPath() + "/mne_scan_plugins/resources/babymeg/both.bad")
, m_iRecordingMSeconds(5*60*1000)
, m_dSfreq(1024)
, m_iSplitCount(0)
{
    m_pActionSetupProject = new QAction(QIcon(":/images/database.png"), tr("Setup Project"),this);
//    m_pActionSetupProject->setShortcut(tr("F12"));
    m_pActionSetupProject->setStatusTip(tr("Setup Project"));
    connect(m_pActionSetupProject, &QAction::triggered,
            this, &BabyMEG::showProjectDialog);
    addPluginAction(m_pActionSetupProject);    

    m_pActionRecordFile = new QAction(QIcon(":/images/record.png"), tr("Start Recording"),this);
    m_pActionRecordFile->setStatusTip(tr("Start Recording"));
    connect(m_pActionRecordFile, &QAction::triggered,
            this, &BabyMEG::toggleRecordingFile);
    addPluginAction(m_pActionRecordFile);
    //m_pActionRecordFile->setEnabled(false);

    m_pActionSqdCtrl = new QAction(QIcon(":/images/sqdctrl.png"), tr("Squid Control"),this);
//    m_pActionSetupProject->setShortcut(tr("F12"));
    m_pActionSqdCtrl->setStatusTip(tr("Squid Control"));
    connect(m_pActionSqdCtrl, &QAction::triggered,
            this, &BabyMEG::showSqdCtrlDialog);
    addPluginAction(m_pActionSqdCtrl);

    m_pActionUpdateFiffInfo = new QAction(QIcon(":/images/latestFiffInfo.png"), tr("Update Fiff Info"),this);
    m_pActionUpdateFiffInfo->setStatusTip(tr("Update Fiff Info"));
    connect(m_pActionUpdateFiffInfo, &QAction::triggered,
            this, &BabyMEG::UpdateFiffInfo);
    addPluginAction(m_pActionUpdateFiffInfo);

    m_pActionUpdateFiffInfoForHPI = new QAction(QIcon(":/images/latestFiffInfoHPI.png"), tr("Update HPI to Fiff Info"),this);
    m_pActionUpdateFiffInfoForHPI->setStatusTip(tr("Update HPI to Fiff Info"));
    connect(m_pActionUpdateFiffInfoForHPI, &QAction::triggered,
            this, &BabyMEG::SetFiffInfoForHPI);
    addPluginAction(m_pActionUpdateFiffInfoForHPI);

    //Init timers
    if(!m_pRecordTimer) {
        m_pRecordTimer = QSharedPointer<QTimer>(new QTimer(this));
        m_pRecordTimer->setSingleShot(true);
        connect(m_pRecordTimer.data(), &QTimer::timeout,
                this, &BabyMEG::toggleRecordingFile);
    }

    if(!m_pBlinkingRecordButtonTimer) {
        m_pBlinkingRecordButtonTimer = QSharedPointer<QTimer>(new QTimer(this));
        connect(m_pBlinkingRecordButtonTimer.data(), &QTimer::timeout,
                this, &BabyMEG::changeRecordingButton);
    }

    if(!m_pUpdateTimeInfoTimer) {
        m_pUpdateTimeInfoTimer = QSharedPointer<QTimer>(new QTimer(this));
        connect(m_pUpdateTimeInfoTimer.data(), &QTimer::timeout,
                this, &BabyMEG::onRecordingRemainingTimeChange);
    }

    //If the basic MNE Scan version is to be build hide the HPI and squid control actions in the toolbar
    #ifdef BUILD_BASIC_MNESCAN_VERSION
    m_pActionSqdCtrl->setVisible(false);
    m_pActionUpdateFiffInfoForHPI->setVisible(false);
    #endif
}


//*************************************************************************************************************

BabyMEG::~BabyMEG()
{
    if(this->isRunning())
        stop();

    if(m_pMyClient && m_pMyClient->isConnected())
            m_pMyClient->DisConnectBabyMEG();

}


//*************************************************************************************************************

QSharedPointer<IPlugin> BabyMEG::clone() const
{
    QSharedPointer<BabyMEG> pBabyMEGClone(new BabyMEG());
    return pBabyMEGClone;
}


//*************************************************************************************************************

QString BabyMEG::getFilePath(bool currentTime) const
{
    QString sFilePath = m_sBabyMEGDataPath + "/" + m_sCurrentProject + "/" + m_sCurrentSubject;

    QString sTimeStamp;

    if(currentTime)
        sTimeStamp = QDateTime::currentDateTime().toString("yyMMdd_hhmmss");
    else
        sTimeStamp = "<YYMMDD_HMS>";

    if(m_sCurrentParadigm.isEmpty())
        sFilePath.append("/"+ sTimeStamp + "_" + m_sCurrentSubject + "_raw.fif");
    else
        sFilePath.append("/"+ sTimeStamp + "_" + m_sCurrentSubject + "_" + m_sCurrentParadigm + "_raw.fif");

    return sFilePath;
}


//*************************************************************************************************************

QString BabyMEG::getDataPath() const
{
    return m_sBabyMEGDataPath;
}


//*************************************************************************************************************

void BabyMEG::init()
{
    //BabyMEGData Path
    m_sBabyMEGDataPath = QDir::homePath() + "/BabyMEGData";
    if(!QDir(m_sBabyMEGDataPath).exists())
        QDir().mkdir(m_sBabyMEGDataPath);
    //Test Project
    if(!QDir(m_sBabyMEGDataPath+"/TestProject").exists())
        QDir().mkdir(m_sBabyMEGDataPath+"/TestProject");
    QSettings settings;
    m_sCurrentProject = settings.value(QString("Plugin/%1/currentProject").arg(getName()), "TestProject").toString();
    //Test Subject
    if(!QDir(m_sBabyMEGDataPath+"/TestProject/TestSubject").exists())
        QDir().mkdir(m_sBabyMEGDataPath+"/TestProject/TestSubject");
    m_sCurrentSubject = settings.value(QString("Plugin/%1/currentSubject").arg(getName()), "TestSubject").toString();

    //BabyMEG Inits
    pInfo = QSharedPointer<BabyMEGInfo>(new BabyMEGInfo());
    connect(pInfo.data(), &BabyMEGInfo::fiffInfoAvailable,
            this, &BabyMEG::setFiffInfo);
    connect(pInfo.data(), &BabyMEGInfo::SendDataPackage,
            this, &BabyMEG::setFiffData);
    connect(pInfo.data(), &BabyMEGInfo::SendCMDPackage,
            this, &BabyMEG::setCMDData);
    connect(pInfo.data(), &BabyMEGInfo::GainInfoUpdate,
            this, &BabyMEG::setFiffGainInfo);

    m_pMyClient = QSharedPointer<BabyMEGClient>(new BabyMEGClient(6340,this));
    m_pMyClient->SetInfo(pInfo);
    m_pMyClient->start();
    m_pMyClientComm = QSharedPointer<BabyMEGClient>(new BabyMEGClient(6341,this));
    m_pMyClientComm->SetInfo(pInfo);
    m_pMyClientComm->start();

    m_pMyClientComm->SendCommandToBabyMEGShortConnection("INFO");

    m_pMyClient->ConnectToBabyMEG();

    //init channels when fiff info is available
    connect(this, &BabyMEG::fiffInfoAvailable, this, &BabyMEG::initConnector);

    //Init projection dialog
    m_pBabyMEGProjectDialog = QSharedPointer<BabyMEGProjectDialog>(new BabyMEGProjectDialog(this));
}


//*************************************************************************************************************

void BabyMEG::unload()
{

}


//*************************************************************************************************************

void BabyMEG::initConnector()
{
    if(m_pFiffInfo)
    {
        m_pRTMSABabyMEG = PluginOutputData<NewRealTimeMultiSampleArray>::create(this, "BabyMEG Output", "BabyMEG");
        m_pRTMSABabyMEG->data()->setName(this->getName());//Provide name to auto store widget settings

        m_pRTMSABabyMEG->data()->initFromFiffInfo(m_pFiffInfo);
        m_pRTMSABabyMEG->data()->setMultiArraySize(1);

        m_pRTMSABabyMEG->data()->setSamplingRate(m_pFiffInfo->sfreq);

        m_pRTMSABabyMEG->data()->setVisibility(true);

        m_outputConnectors.append(m_pRTMSABabyMEG);

        //Look for trigger channels and initialise detected trigger map
        m_lTriggerChannelIndices.append(m_pFiffInfo->ch_names.indexOf("TRG001"));
        m_lTriggerChannelIndices.append(m_pFiffInfo->ch_names.indexOf("TRG002"));
        m_lTriggerChannelIndices.append(m_pFiffInfo->ch_names.indexOf("TRG003"));
        m_lTriggerChannelIndices.append(m_pFiffInfo->ch_names.indexOf("TRG004"));
        m_lTriggerChannelIndices.append(m_pFiffInfo->ch_names.indexOf("TRG005"));
        m_lTriggerChannelIndices.append(m_pFiffInfo->ch_names.indexOf("TRG006"));
        m_lTriggerChannelIndices.append(m_pFiffInfo->ch_names.indexOf("TRG007"));
        m_lTriggerChannelIndices.append(m_pFiffInfo->ch_names.indexOf("TRG008"));
    }
}


//*************************************************************************************************************

void BabyMEG::clear()
{
    m_pFiffInfo.reset();
    m_iBufferSize = -1;
}


//*************************************************************************************************************

void BabyMEG::showProjectDialog()
{
    m_pBabyMEGProjectDialog->setWindowFlags(Qt::WindowStaysOnTopHint);

    connect(m_pBabyMEGProjectDialog.data(), &BabyMEGProjectDialog::timerChanged,
            this, &BabyMEG::setRecordingTimerChanged);

    connect(m_pBabyMEGProjectDialog.data(), &BabyMEGProjectDialog::recordingTimerStateChanged,
            this, &BabyMEG::setRecordingTimerStateChanged);

    m_pBabyMEGProjectDialog->show();
}


//*************************************************************************************************************

void BabyMEG::showSqdCtrlDialog()
{
    //BabyMEGSQUIDControlDgl SQUIDCtrlDlg(this);
    //SQUIDCtrlDlg.exec();
    // added by Limin for nonmodal dialog
    if (SQUIDCtrlDlg == NULL)
        SQUIDCtrlDlg = QSharedPointer<BabyMEGSQUIDControlDgl>(new BabyMEGSQUIDControlDgl(this));

    if (!SQUIDCtrlDlg->isVisible())
    {
        SQUIDCtrlDlg->show();
        SQUIDCtrlDlg->raise();
        SQUIDCtrlDlg->Init();
    }
}


//*************************************************************************************************************

void BabyMEG::UpdateFiffInfo()
{
    // read gain info and save them to the m_pFiffInfo.range
    m_pMyClientComm->SendCommandToBabyMEGShortConnection("INFG");

    //sleep(0.5);

    //m_pActionRecordFile->setEnabled(true);

}


//*************************************************************************************************************

void BabyMEG::SetFiffInfoForHPI()
{
    if(!m_pFiffInfo)
    {
        QMessageBox msgBox;
        msgBox.setText("FiffInfo missing!");
        msgBox.exec();
        return;
    }
    else
    {
        qDebug()<<" Start to load Polhemus File";
        if (HPIDlg == NULL)
            HPIDlg = QSharedPointer<BabyMEGHPIDgl>(new BabyMEGHPIDgl(this));

        if (!HPIDlg->isVisible())
        {
            HPIDlg->show();
            HPIDlg->raise();
        }
    }
}


//*************************************************************************************************************

void BabyMEG::RecvHPIFiffInfo(const FiffInfo& info)
{
    // show the HPI info
    qDebug()<<"saved HPI" << m_pFiffInfo->dig.at(0).r[0];
    qDebug()<<"HPI"<< info.dig.at(0).kind << info.dig.at(0).r[0];
}


//*************************************************************************************************************

void BabyMEG::splitRecordingFile()
{
    qDebug() << "Split recording file";
    ++m_iSplitCount;
    QString nextFileName = m_sRecordFile.remove("_raw.fif");
    nextFileName += QString("-%1_raw.fif").arg(m_iSplitCount);

    /*
    * Write the link to the next file
    */
    qint32 data;
    m_pOutfid->start_block(FIFFB_REF);
    data = FIFFV_ROLE_NEXT_FILE;
    m_pOutfid->write_int(FIFF_REF_ROLE,&data);
    m_pOutfid->write_string(FIFF_REF_FILE_NAME, nextFileName);
    m_pOutfid->write_id(FIFF_REF_FILE_ID);//ToDo meas_id
    data = m_iSplitCount - 1;
    m_pOutfid->write_int(FIFF_REF_FILE_NUM, &data);
    m_pOutfid->end_block(FIFFB_REF);

    //finish file
    m_pOutfid->finish_writing_raw();

    //start next file
    m_qFileOut.setFileName(nextFileName);
    m_pOutfid = FiffStream::start_writing_raw(m_qFileOut, *m_pFiffInfo, m_cals, defaultMatrixXi, false);
    fiff_int_t first = 0;
    m_pOutfid->write_int(FIFF_FIRST_SAMPLE, &first);
}


//*************************************************************************************************************

void BabyMEG::toggleRecordingFile()
{
    //Setup writing to file
    if(m_bWriteToFile)
    {
        mutex.lock();
        m_pOutfid->finish_writing_raw();
        mutex.unlock();

        m_bWriteToFile = false;
        m_iSplitCount = 0;

        //Stop record timer
        m_pRecordTimer->stop();
        m_pUpdateTimeInfoTimer->stop();
        m_pBlinkingRecordButtonTimer->stop();

        m_pActionRecordFile->setIcon(QIcon(":/images/record.png"));
    }
    else
    {
        m_iSplitCount = 0;

        if(!m_pFiffInfo)
        {
            QMessageBox msgBox;
            msgBox.setText("FiffInfo missing!");
            msgBox.exec();
            return;
        }

        //Initiate the stream for writing to the fif file
        m_sRecordFile = getFilePath(true);
        m_qFileOut.setFileName(m_sRecordFile);
        if(m_qFileOut.exists())
        {
            QMessageBox msgBox;
            msgBox.setText("The file you want to write already exists.");
            msgBox.setInformativeText("Do you want to overwrite this file?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            int ret = msgBox.exec();
            if(ret == QMessageBox::No)
                return;
        }

        //Set all projectors to zero before writing to file because we always write the raw data
        for(int i = 0; i<m_pFiffInfo->projs.size(); i++)
            m_pFiffInfo->projs[i].active = false;

        mutex.lock();
        m_pOutfid = FiffStream::start_writing_raw(m_qFileOut, *m_pFiffInfo, m_cals, defaultMatrixXi, false);
        fiff_int_t first = 0;
        m_pOutfid->write_int(FIFF_FIRST_SAMPLE, &first);
        mutex.unlock();

        m_bWriteToFile = true;

        //Start timers for record button blinking, recording timer and updating the elapsed time in the proj widget
        m_pBlinkingRecordButtonTimer->start(500);
        m_recordingStartedTime.restart();
        m_pUpdateTimeInfoTimer->start(1000);

        if(m_bUseRecordTimer)
            m_pRecordTimer->start(m_iRecordingMSeconds);
    }
}


//*************************************************************************************************************

void BabyMEG::setFiffData(QByteArray DATA)
{
    //get the first byte -- the data format
    int dformat = DATA.left(1).toInt();

    DATA.remove(0,1);
    qint32 rows = m_pFiffInfo->nchan;
    qint32 cols = (DATA.size()/dformat)/rows;

    qDebug() << "[BabyMEG] Matrix " << rows << "x" << cols << " [Data bytes:" << dformat << "]";

    MatrixXf rawData(Map<MatrixXf>( (float*)DATA.data(),rows, cols ));

    for(qint32 i = 0; i < rows*cols; ++i)
        IOUtils::swap_floatp(rawData.data()+i);


    if(m_bIsRunning)
    {
        if(!m_pRawMatrixBuffer)
            m_pRawMatrixBuffer = CircularMatrixBuffer<float>::SPtr(new CircularMatrixBuffer<float>(40, rows, cols));

        m_pRawMatrixBuffer->push(&rawData);
    }
//    else
//    {
////        std::cout << "Data coming" << std::endl; //"first ten elements \n" << rawData.block(0,0,1,10) << std::endl;

//        emit DataToSquidCtrlGUI(rawData);
//    }

    emit DataToSquidCtrlGUI(rawData);
}


//*************************************************************************************************************

void BabyMEG::setFiffInfo(const FiffInfo& p_FiffInfo)
{
    m_pFiffInfo = QSharedPointer<FiffInfo>(new FiffInfo(p_FiffInfo));

    if(!readProjectors())
    {
        qDebug() << "Not able to read projectors";
    }

    if(!readCompensators())
    {
        qDebug() << "Not able to read compensators";
    }

    if(!readBadChannels())
    {
        qDebug() << "Not able to read bad channels";
    }

    m_iBufferSize = pInfo->dataLength;
    m_dSfreq = pInfo->sfreq;

    //
    //   Add the calibration factors
    //
    m_cals = RowVectorXd(m_pFiffInfo->nchan);
    m_cals.setZero();
    for (qint32 k = 0; k < m_pFiffInfo->nchan; ++k)
        m_cals[k] = m_pFiffInfo->chs[k].range*m_pFiffInfo->chs[k].cal;

    //
    //  Initialize the data and calibration vector
    //
    typedef Eigen::Triplet<double> T;
    std::vector<T> tripletList;
    tripletList.reserve(m_pFiffInfo->nchan);
    for(qint32 i = 0; i < m_pFiffInfo->nchan; ++i)
        tripletList.push_back(T(i, i, this->m_cals[i]));

    m_sparseMatCals = SparseMatrix<double>(m_pFiffInfo->nchan, m_pFiffInfo->nchan);
    m_sparseMatCals.setFromTriplets(tripletList.begin(), tripletList.end());

    emit fiffInfoAvailable();
}


//*************************************************************************************************************

void BabyMEG::setFiffGainInfo(QStringList GainInfo)
{
    if(!m_pFiffInfo)
    {
        QMessageBox msgBox;
        msgBox.setText("FiffInfo missing!");
        msgBox.exec();
        return;
    }
    else
    {
        //set up the gain info
        qDebug()<<"Set Gain Info";
        for(qint32 i = 0; i < m_pFiffInfo->nchan; i++) {
            m_pFiffInfo->chs[i].range = 1.0f/GainInfo.at(i).toFloat();//1; // set gain
            m_cals[i] = m_pFiffInfo->chs[i].range*m_pFiffInfo->chs[i].cal;
            //qDebug()<<i<<"="<<m_pFiffInfo->chs[i].ch_name<<","<<m_pFiffInfo->chs[i].range;
        }

        // Initialize the data and calibration vector
        typedef Eigen::Triplet<double> T;
        std::vector<T> tripletList;
        tripletList.reserve(m_pFiffInfo->nchan);
        for(qint32 i = 0; i < m_pFiffInfo->nchan; ++i)
            tripletList.push_back(T(i, i, this->m_cals[i]));

        m_sparseMatCals = SparseMatrix<double>(m_pFiffInfo->nchan, m_pFiffInfo->nchan);
        m_sparseMatCals.setFromTriplets(tripletList.begin(), tripletList.end());
    }

}


//*************************************************************************************************************

void BabyMEG::setCMDData(QByteArray DATA)
{
    qDebug()<<"------"<<DATA;
//    m_commandManager["FLL"].reply(DATA);
    emit SendCMDDataToSQUIDControl(DATA);
    qDebug()<<"Data has been received.";
}


//*************************************************************************************************************
void BabyMEG::comFLL(QString t_sFLLControlCommand)
{
    qDebug()<<"FLL commands";

    qDebug() << "BabyMeg Received" << t_sFLLControlCommand;
    int strlen = t_sFLLControlCommand.size();
    QByteArray Scmd = m_pMyClientComm->MGH_LM_Int2Byte(strlen);
    QByteArray SC = QByteArray("COMS")+Scmd;
    SC.append(t_sFLLControlCommand);
    m_pMyClientComm->SendCommandToBabyMEGShortConnection(SC);
}


//*************************************************************************************************************

bool BabyMEG::start()
{
    //Check if the thread is already or still running. This can happen if the start button is pressed immediately after the stop button was pressed. In this case the stopping process is not finished yet but the start process is initiated.
    if(this->isRunning())
        QThread::wait();

    if(!m_pRTMSABabyMEG)
        initConnector();

    // Start threads
    m_bIsRunning = true;

    if(!m_pMyClient->isConnected())
        m_pMyClient->ConnectToBabyMEG();
    // Start threads
    QThread::start();

    return true;
}


//*************************************************************************************************************

bool BabyMEG::stop()
{
    if(m_pMyClient->isConnected())
        m_pMyClient->DisConnectBabyMEG();

    m_bIsRunning = false;

    //In case the semaphore blocks the thread -> Release the QSemaphore and let it exit from the pop function (acquire statement)
    m_pRawMatrixBuffer->releaseFromPop();

    //Clear Buffers
    m_pRawMatrixBuffer->clear();

    return true;
}


//*************************************************************************************************************

IPlugin::PluginType BabyMEG::getType() const
{
    return _ISensor;
}


//*************************************************************************************************************

QString BabyMEG::getName() const
{
    return "BabyMEG";
}


//*************************************************************************************************************

QWidget* BabyMEG::setupWidget()
{
    if(!m_pMyClient->isConnected())
        m_pMyClient->ConnectToBabyMEG();

    BabyMEGSetupWidget* widget = new BabyMEGSetupWidget(this);//widget is later distroyed by CentralWidget - so it has to be created everytime new

    //init dialog

    return widget;
}


//*************************************************************************************************************

void BabyMEG::run()
{
    MatrixXf matValue;

    qint32 size = 0;

    while(m_bIsRunning)
    {
        if(m_pRawMatrixBuffer)
        {
            //pop matrix
            matValue = m_pRawMatrixBuffer->pop();

            //create digital trigger information
            //QElapsedTimer time;
            //time.start();
            createDigTrig(matValue);
            //qDebug()<<"BabyMEG::run - createDigTrig took: "<<time.elapsed();

            //Write raw data to fif file
            if(m_bWriteToFile)
            {
                size += matValue.rows()*matValue.cols() * 4;

                if(size > MAX_DATA_LEN)
                {
                    size = 0;
                    this->splitRecordingFile();
                }

                mutex.lock();
                m_pOutfid->write_raw_buffer(matValue.cast<double>());
                mutex.unlock();
            }
            else
            {
                size = 0;
            }

            if(m_pRTMSABabyMEG)
            {
                m_pRTMSABabyMEG->data()->setValue(this->calibrate(matValue));
            }
        }
    }

    //Close the fif output stream
    if(m_bWriteToFile)
    {
        this->toggleRecordingFile();
    }
}


//*************************************************************************************************************

void BabyMEG::createDigTrig(MatrixXf& data)
{
    //Look for triggers in all trigger channels

    //m_qMapDetectedTrigger = DetectTrigger::detectTriggerFlanksMax(data.at(b), m_lTriggerChannelIndices, m_iCurrentSample-nCol, m_dTriggerThreshold, true);
    QMap<int,QList<QPair<int,double> > > qMapDetectedTrigger = DetectTrigger::detectTriggerFlanksGrad(data.cast<double>(), m_lTriggerChannelIndices, 0, 3.0, false, "Rising");

    //Combine and write results into data block's digital trigger channel
    QMapIterator<int,QList<QPair<int,double> >> i(qMapDetectedTrigger);
    int counter = 0;
    int idxDigTrig = m_pFiffInfo->ch_names.indexOf("DTRG01");

    while (i.hasNext())
    {
        i.next();

        QList<QPair<int,double> > lDetectedTriggers = i.value();

        for(int k = 0; k < lDetectedTriggers.size(); ++k)
        {
            if(lDetectedTriggers.at(k).first < data.cols() && lDetectedTriggers.at(k).first >= 0)
            {
                data(idxDigTrig,lDetectedTriggers.at(k).first) = data(idxDigTrig,lDetectedTriggers.at(k).first) + pow(2,counter);
            }
        }

        counter++;
    }
}


//*************************************************************************************************************

MatrixXd BabyMEG::calibrate(const MatrixXf& data)
{
    MatrixXd one;
    if(m_pFiffInfo && m_sparseMatCals.cols() == m_pFiffInfo->nchan)
        one = m_sparseMatCals*data.cast<double>();
    else
        one = data.cast<double>();

    return one;
}


//*************************************************************************************************************

bool BabyMEG::readProjectors()
{
    QFile t_projFiffFile(m_sFiffProjections);

    //
    //   Open the file
    //
    FiffStream::SPtr t_pStream(new FiffStream(&t_projFiffFile));
    QString t_sFileName = t_pStream->streamName();

    printf("Opening header data %s...\n",t_sFileName.toUtf8().constData());

    FiffDirTree t_Tree;
    QList<FiffDirEntry> t_Dir;

    if(!t_pStream->open(t_Tree, t_Dir))
        return false;

    QList<FiffProj> q_ListProj = t_pStream->read_proj(t_Tree);

    //Set all projectors to zero
    for(int i = 0; i<q_ListProj.size(); i++)
        q_ListProj[i].active = false;

    if (q_ListProj.size() == 0)
    {
        printf("Could not find projectors\n");
        return false;
    }

    m_pFiffInfo->projs = q_ListProj;

    //garbage collecting
    t_pStream->device()->close();

    return true;
}


//*************************************************************************************************************

bool BabyMEG::readCompensators()
{
    QFile t_compFiffFile(m_sFiffCompensators);

    //
    //   Open the file
    //
    FiffStream::SPtr t_pStream(new FiffStream(&t_compFiffFile));
    QString t_sFileName = t_pStream->streamName();

    printf("Opening compensator data %s...\n",t_sFileName.toUtf8().constData());

    FiffDirTree t_Tree;
    QList<FiffDirEntry> t_Dir;

    if(!t_pStream->open(t_Tree, t_Dir))
        return false;

    QList<FiffCtfComp> q_ListComp = t_pStream->read_ctf_comp(t_Tree, m_pFiffInfo->chs);

    if (q_ListComp.size() == 0)
    {
        printf("Could not find compensators\n");
        return false;
    }

    m_pFiffInfo->comps = q_ListComp;

    //garbage collecting
    t_pStream->device()->close();

    return true;
}


//*************************************************************************************************************

bool BabyMEG::readBadChannels()
{
    //
    // Bad Channels
    //
//    //Read bad channels from header/projection fif
//    QFile t_headerFiffFile(m_sFiffProjections);

//    if(!t_headerFiffFile.exists()) {
//        printf("Could not open fif file for copying bad channels to babyMEG fiff_info\n");
//        return false;
//    }

//    FiffRawData raw(t_headerFiffFile);
//    m_pFiffInfo->bads = raw.info.bads;

//    t_headerFiffFile.close();

//    return true;

    //Read bad channels from
    QFile t_badChannelsFile(m_sBadChannels);

    if (!t_badChannelsFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    printf("Reading bad channels from %s...\n", m_sBadChannels.toUtf8().constData());

    QTextStream in(&t_badChannelsFile);
    qint32 count = 0;
    QStringList t_sListbads;
    while (!in.atEnd()) {
        QString channel = in.readLine();
        if(channel.isEmpty())
            continue;
        ++count;
        printf("Channel %i: %s\n",count,channel.toUtf8().constData());
        t_sListbads << channel;
    }

    m_pFiffInfo->bads = t_sListbads;

    return true;
}


//*************************************************************************************************************

void BabyMEG::changeRecordingButton()
{
    if(m_iBlinkStatus == 0)
    {
        m_pActionRecordFile->setIcon(QIcon(":/images/record.png"));
        m_iBlinkStatus = 1;
    }
    else
    {
        m_pActionRecordFile->setIcon(QIcon(":/images/record_active.png"));
        m_iBlinkStatus = 0;
    }
}


//*************************************************************************************************************

void BabyMEG::setRecordingTimerChanged(int timeMSecs)
{
    //If the recording time is changed during the recording, change the timer
    if(m_bWriteToFile)
        m_pRecordTimer->setInterval(timeMSecs-m_recordingStartedTime.elapsed());

    m_iRecordingMSeconds = timeMSecs;
}


//*************************************************************************************************************

void BabyMEG::setRecordingTimerStateChanged(bool state)
{
    m_bUseRecordTimer = state;
}


//*************************************************************************************************************

void BabyMEG::onRecordingRemainingTimeChange()
{
    m_pBabyMEGProjectDialog->setRecordingElapsedTime(m_recordingStartedTime.elapsed());
}


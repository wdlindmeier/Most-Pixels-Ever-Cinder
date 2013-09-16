//
//  MPEClient.cpp
//  Unknown Project
//
//  Copyright (c) 2013 William Lindmeier. All rights reserved.
//

#define BOOST_BIND_NO_PLACEHOLDERS

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused"
#include <boost/lambda/lambda.hpp>
#pragma clang diagnostic pop
#include "cinder/app/App.h"
#include "cinder/Camera.h"
#include "cinder/CinderResources.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/Vector.h"
#include "cinder/Xml.h"
#include "MPEClient.h"
#include "MPEProtocol.hpp"
#include "TCPAsyncClient.h"

using std::string;
using std::vector;
using namespace ci;
using namespace ci::app;
using namespace mpe;

namespace mpe
{
    class MPENonThreadedClient : public MPEClient
    {

    protected:

        // A pointer to your Cinder app
        MPEApp                          *mApp;

        // The version of MPE you're using.
        boost::shared_ptr<MPEProtocol>  mProtocol;

        bool                            mIsRendering3D;
        long                            mLastFrameConfirmed;

        // 3D Positioning
        float                           mFieldOfView;
        float                           mCameraZ;
        float                           mAspectRatio;

        // Settings loaded from settings.xml
        int                             mPort;
        std::string                     mHostname;
        bool                            mIsStarted;
        ci::Rectf                       mLocalViewportRect;
        ci::Vec2i                       mMasterSize;
        int                             mClientID;
        bool                            mIsDebug;
        bool                            mIsAsync;
        std::string                     mClientName;
        bool                            mAsyncReceivesData;

        // A connection to the server.
        boost::shared_ptr<TCPClient>    mTCPClient;

        ci::CameraPersp                 mCamera3D;

    public:

        typedef boost::shared_ptr<MPENonThreadedClient> Ptr;

        MPENonThreadedClient(MPEApp *cinderApp) :
        MPEClient(),
        mHostname(""),
        mPort(0),
        mIsStarted(false),
        mIsRendering3D(false),
        mClientID(-1),
        mIsDebug(false),
        mLastFrameConfirmed(-1),
        mIsAsync(false),
        mAsyncReceivesData(false),
        mClientName(""),
        mAspectRatio(getWindowAspectRatio()),
        mFieldOfView(25.0f),
        mCameraZ(-880)
        {
            mApp = cinderApp;
            mProtocol = mApp->mpeProtocol();
            loadSettings(mApp->mpeSettingsFile());
        }

        ~MPENonThreadedClient(){ stop(); }

        #pragma mark - Accessors

        virtual bool isThreaded()
        {
            return false;
        }

        int getClientID()
        {
            return mClientID;
        }

        string getClientName()
        {
            return mClientName;
        }

        ci::Rectf getVisibleRect()
        {
            return mLocalViewportRect;
        }

        void setVisibleRect(const ci::Rectf & rect)
        {
            mLocalViewportRect = rect;
        }

        ci::Vec2i getMasterSize()
        {
            return mMasterSize;
        }

        bool getIsRendering3D()
        {
            return mIsRendering3D;
        }

        void setIsRendering3D(bool is3D)
        {
            mIsRendering3D = is3D;
        }

        bool isAsynchronousClient()
        {
            return mIsAsync;
        }

        #pragma mark - Connection

        void start(const string & hostname, const int port)
        {
            mHostname = hostname;
            mPort = port;
            start();
        }

        virtual void start()
        {
            if (mIsStarted)
            {
                stop();
            }

            mIsStarted = true;
            mTCPClient = boost::shared_ptr<TCPClient>(new TCPClient(mProtocol->incomingMessageDelimiter()));

            if (mTCPClient->open(mHostname, mPort))
            {
                tcpDidConnect();
            }
            else
            {
                stop();
            }
        }

    protected:

        void tcpDidConnect()
        {
            if (mIsDebug)
            {
                console() << "Established synchronous connection to server: "
                          << mHostname << ":" << mPort << std::endl;
            }
            sendClientID();
        }

    public:

        virtual void stop()
        {
            mIsStarted = false;
            if (mTCPClient)
            {
                mTCPClient->close();
                mTCPClient = NULL;
            }
        }

        void togglePause()
        {
            mTCPClient->write(mProtocol->togglePause());
        }

        void resetAll()
        {
            mTCPClient->write(mProtocol->resetAll());
        }

        bool isConnected()
        {
            return mIsStarted && mTCPClient && mTCPClient->isConnected();
        }

        #pragma mark - Update

        virtual void update()
        {
            mFrameIsReady = false;

            if (mIsStarted && isConnected())
            {
                bool isDataAvailable = true;

                string data = mTCPClient->read(isDataAvailable);
                if (isDataAvailable)
                {
                    // There may be more than 1 message in the read.
                    std::vector<string> messages = ci::split(data,
                                                             mProtocol->incomingMessageDelimiter());
                    for (int i = 0; i < messages.size(); ++i)
                    {
                        std::string message = messages[i];
                        if (message.length() > 0)
                        {
                            mProtocol->parse(message, this);
                        }
                    }
                }

                if (mFrameIsReady)
                {
                    mApp->mpeFrameUpdate(this->getCurrentRenderFrame());
                }
            }
        }

        #pragma mark - Drawing

        virtual void draw()
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(0, 0,
                      mLocalViewportRect.getWidth(),
                      mLocalViewportRect.getHeight());
            
            glPushMatrix();

            // Only show the area of the view we're interested in.
            positionViewport();

            // Tell the app to draw.
            mApp->mpeFrameRender(mFrameIsReady);

            glPopMatrix();
            
            gl::disable(GL_SCISSOR_TEST);

            if (isConnected() && !mIsAsync)
            {
                // Tell the server we're ready for the next.
                // NOTE: Async clients must not report their render progress to the server.
                doneRendering();
            }
        }

        void positionViewport()
        {
            if (mIsRendering3D)
            {
                positionViewport3D();
            }
            else
            {
                positionViewport2D();
            }
        }

        // 2D Positioning
        void positionViewport2D()
        {
            gl::setMatricesWindow(mLocalViewportRect.getWidth(),
                                  mLocalViewportRect.getHeight());
            glTranslatef(mLocalViewportRect.getX1() * -1,
                         mLocalViewportRect.getY1() * -1,
                         0);
        }

        // 3D Positioning
        void positionViewport3D()
        {
            float mWidth = mMasterSize.x;
            float mHeight = mMasterSize.y;
            float lWidth = mLocalViewportRect.getWidth();
            float lHeight = mLocalViewportRect.getHeight();
            float xOffset = mLocalViewportRect.getX1();
            float yOffset = mLocalViewportRect.getY1();

            mCamera3D.setPerspective(mFieldOfView,
                                     mAspectRatio,
                                     1.0f, // near
                                     10000.0f); // far

            Vec3f eye(mWidth/2.f, mHeight/2.f, mCameraZ);
            Vec3f target(mWidth/2.f, mHeight/2.f, 0);
            Vec3f up(0, -1, 0);
            mCamera3D.lookAt(eye, target, up);

            float horizCenterMaster = mWidth / 2.0f;
            float horizCenterView = xOffset + (lWidth * 0.5);
            float horizPxShift = horizCenterMaster - horizCenterView;
            float horizOffset = (horizPxShift / lWidth) * -2.0f;

            float vertCenterMaster = mHeight / 2.0f;
            float vertCenterView = yOffset + (lHeight * 0.5);
            float vertPxShift = vertCenterMaster - vertCenterView;
            float vertOffset = (vertPxShift / lHeight) * 2.0f;

            mCamera3D.setLensShift(horizOffset, vertOffset);
            gl::setMatrices(mCamera3D);
        }

        void set3DFieldOfView(float fov)
        {
            mFieldOfView = fov;
        }

        float get3DFieldOfView()
        {
            return mFieldOfView;
        }

        void set3DCameraZ(float camZ)
        {
            mCameraZ = camZ;
        }

        float get3DCameraZ()
        {
            return mCameraZ;
        }

        void set3DAspectRatio(float aspectRatio)
        {
            mAspectRatio = aspectRatio;
        }

        float get3DAspectRatio()
        {
            return mAspectRatio;
        }

        #pragma mark - Hit Testing

        // TODO: These should account for 3D

        bool isOnScreen(const Vec2f & pos)
        {
            return isOnScreen(pos.x, pos.y);
        }

        bool isOnScreen(float x, float y)
        {
            float lWidth = mLocalViewportRect.getWidth();
            float lHeight = mLocalViewportRect.getHeight();
            float xOffset = mLocalViewportRect.getX1();
            float yOffset = mLocalViewportRect.getY1();
            return (x > xOffset &&
                    x < (xOffset + lWidth) &&
                    y > yOffset &&
                    y < (yOffset + lHeight));
        }

        bool isOnScreen(const Rectf & rect)
        {
            return isOnScreen(rect.x1, rect.y1, rect.getWidth(), rect.getHeight());
        }

        bool isOnScreen(float x, float y, float w, float h)
        {
            return (isOnScreen(x, y) ||
                    isOnScreen(x + w, y) ||
                    isOnScreen(x + w, y + h) ||
                    isOnScreen(x, y + h));
        }

        #pragma mark - Sending Messages

    protected:

        void sendClientID()
        {
            if (mIsAsync)
            {
                mTCPClient->write(mProtocol->setAsyncClientID(mClientID,
                                                              mClientName,
                                                              mAsyncReceivesData));
            }
            else
            {
                mTCPClient->write(mProtocol->setClientID(mClientID, mClientName));
            }
        }

    public:

        void sendMessage(const std::string & message)
        {
            mTCPClient->write(mProtocol->broadcast(message));
        }

        void sendMessage(const std::string & message, const std::vector<int> & clientIds)
        {
            mTCPClient->write(mProtocol->broadcast(message, clientIds));
        }

    protected:

        void doneRendering()
        {
            // Only inform the server if this is a new frame. It's possible that a given frame is
            // rendered multiple times if the server update is slower than the app loop.
            if (mLastFrameConfirmed < mCurrentRenderFrame)
            {
                mTCPClient->write(mProtocol->renderIsComplete(mClientID, mCurrentRenderFrame));
                mLastFrameConfirmed = mCurrentRenderFrame;
            }
        }

        #pragma mark - MPEMessageHandler

    public:

        void setCurrentRenderFrame(long frameNum)
        {
            MPEMessageHandler::setCurrentRenderFrame(frameNum);
            // mLastFrameConfirmed has to reset when the current render frame is.
            mLastFrameConfirmed = mCurrentRenderFrame - 1;
        }

        void receivedResetCommand()
        {
            mApp->mpeReset();
        }

        #pragma mark - Receiving Messages

        virtual void receivedStringMessage(const std::string & dataMessage, const int fromClientID)
        {
            mApp->mpeMessageReceived(dataMessage, fromClientID);
        }

        #pragma mark - Settings

    private:

        void loadSettings(DataSourceRef settingsXMLFile)
        {
            XmlTree settingsDoc(settingsXMLFile);

            try
            {
                XmlTree node = settingsDoc.getChild( "settings/asynchronous" );
                string boolStr = node.getValue<string>();
                std::transform(boolStr.begin(), boolStr.end(), boolStr.begin(), ::tolower);
                mIsAsync = (boolStr == "true");
            }
            catch (XmlTree::ExcChildNotFound e)
            {
                // Assume false.
                mIsAsync = false;
            }

            if (mIsAsync)
            {
                try
                {
                    XmlTree node = settingsDoc.getChild( "settings/asynchreceive" );
                    string boolStr = node.getValue<string>();
                    std::transform(boolStr.begin(), boolStr.end(), boolStr.begin(), ::tolower);
                    mAsyncReceivesData = (boolStr == "true");
                }
                catch (XmlTree::ExcChildNotFound e)
                {
                    // Assume false.
                    mAsyncReceivesData = false;
                }
            }

            try
            {
                XmlTree ipNode = settingsDoc.getChild( "settings/client_id" );
                mClientID = ipNode.getValue<int>();
            }
            catch (XmlTree::ExcChildNotFound e)
            {
                console() << "ERROR: Could not find client ID." << std::endl;
            }

            try
            {
                XmlTree node = settingsDoc.getChild( "settings/name" );
                mClientName = node.getValue<string>();
            }
            catch (XmlTree::ExcChildNotFound e)
            {
                if (mIsAsync)
                {
                    mClientName = "Async client " + std::to_string(mClientID);
                }
                else
                {
                    mClientName = "Sync client " + std::to_string(mClientID);
                }
            }

            try
            {
                XmlTree debugNode = settingsDoc.getChild( "settings/debug" );
                mIsDebug = debugNode.getValue<int>();
            }
            catch (XmlTree::ExcChildNotFound e)
            {
                // Ignore
            }

            try
            {
                XmlTree ipNode = settingsDoc.getChild( "settings/server/ip" );
                mHostname = ipNode.getValue<string>();
                mPort = settingsDoc.getChild( "settings/server/port" ).getValue<int>();
            }
            catch (XmlTree::ExcChildNotFound e)
            {
                console() << "ERROR: Could not find server and port settings." << std::endl;
            }

            try
            {
                XmlTree widthNode = settingsDoc.getChild( "settings/local_dimensions/width" );
                XmlTree heightNode = settingsDoc.getChild( "settings/local_dimensions/height" );
                XmlTree xNode = settingsDoc.getChild( "settings/local_location/x" );
                XmlTree yNode = settingsDoc.getChild( "settings/local_location/y" );
                int width = widthNode.getValue<int>();
                int height = heightNode.getValue<int>();
                int x = xNode.getValue<int>();
                int y = yNode.getValue<int>();
                mLocalViewportRect = Rectf( x, y, x+width, y+height );

                // Force the window size based on the settings XML.
                ci::app::setWindowSize(width, height);
            }
            catch (XmlTree::ExcChildNotFound e)
            {
                if (!mIsAsync)
                {
                    // Async controller doesn't need to know about the dimensions
                    console() << "ERROR: Could not find local dimensions settings." << std::endl;
                }
            }

            try
            {
                XmlTree widthNode = settingsDoc.getChild( "settings/master_dimensions/width" );
                XmlTree heightNode = settingsDoc.getChild( "settings/master_dimensions/height" );
                int width = widthNode.getValue<int>();
                int height = heightNode.getValue<int>();
                mMasterSize = Vec2i(width, height);
            }
            catch (XmlTree::ExcChildNotFound e)
            {
                if (!mIsAsync)
                {
                    // Async controller doesn't need to know about the dimensions
                    console() << "ERROR: Could not find master dimensions settings" << std::endl;
                }
            }

            try
            {
                XmlTree fullscreenNode = settingsDoc.getChild("settings/go_fullscreen");
                string boolStr = fullscreenNode.getValue<string>();
                std::transform(boolStr.begin(), boolStr.end(), boolStr.begin(), ::tolower);
                if (boolStr == "true")
                {
                    ci::app::setFullScreen(true);
                }
            }
            catch (XmlTree::ExcChildNotFound e)
            {
                // Not required
            }

            try
            {
                XmlTree fullscreenNode = settingsDoc.getChild("settings/offset_window");
                string boolStr = fullscreenNode.getValue<string>();
                std::transform(boolStr.begin(), boolStr.end(), boolStr.begin(), ::tolower);
                if (boolStr == "true")
                {
                    ci::app::setWindowPos(Vec2i(mLocalViewportRect.x1, mLocalViewportRect.y1));
                }
            }
            catch (XmlTree::ExcChildNotFound e)
            {
                // Not required
            }
        }
    };

    #pragma mark - Threaded

    class MPEThreadedClient : public MPENonThreadedClient
    {

        // This lock is to protect the client data that's being updated
        // on one thread (that's communicating with the server)
        // and accessed for drawing on another thread.
        std::mutex              mClientDataMutex;
        bool                    mShouldUpdate;

    public:

        typedef boost::shared_ptr<MPEThreadedClient> Ptr;

        MPEThreadedClient(MPEApp *cinderApp) :
        MPENonThreadedClient(cinderApp)
        {
        };

        #pragma mark - Accessors

        bool isThreaded()
        {
            return true;
        }

        #pragma mark - Connection

        void start()
        {
            if (mIsStarted)
            {
                stop();
            }

            mIsStarted = true;
            mLastFrameConfirmed = -1;
            mTCPClient = boost::shared_ptr<TCPAsyncClient>(new TCPAsyncClient(mProtocol->incomingMessageDelimiter()));

            boost::shared_ptr<TCPAsyncClient> client = boost::dynamic_pointer_cast<TCPAsyncClient>(mTCPClient);
            client->setIncomingMessageCallback(boost::bind(&MPEThreadedClient::serverMessageReceived,
                                                           this,
                                                           boost::lambda::_1));
            client->open(mHostname, mPort, boost::bind(&MPEThreadedClient::tcpDidConnect,
                                                       this,
                                                       boost::lambda::_1,
                                                       boost::lambda::_2));
        }

    protected:

        void tcpDidConnect(bool didConnect, const boost::system::error_code & error)
        {
            if (didConnect)
            {
                if (mIsDebug)
                {
                    console() << "Established async connection to server: "
                    << mHostname << ":" << mPort << std::endl;
                }
                sendClientID();
            }
            else
            {
                stop();
            }
        }

        #pragma mark - Receiving Messages

        void serverMessageReceived(const std::string & message)
        {
            mFrameIsReady = false;
            // This will set mFrameIsReady
            mProtocol->parse(message, this);
            if (mFrameIsReady)
            {
                std::lock_guard<std::mutex> lock(mClientDataMutex);
                mApp->mpeFrameUpdate(this->getCurrentRenderFrame());
            }
        }

    public:

        void receivedStringMessage(const std::string & dataMessage, const int fromClientID)
        {
            std::lock_guard<std::mutex> lock(mClientDataMutex);
            MPENonThreadedClient::receivedStringMessage(dataMessage, fromClientID);
        }

        void receivedResetCommand()
        {
            std::lock_guard<std::mutex> lock(mClientDataMutex);
            MPENonThreadedClient::receivedResetCommand();
        }

        #pragma mark - Update

        void update()
        {
            if (mIsDebug)
            {
                static bool DidAlertAsyncNoEffect = false;
                if (!DidAlertAsyncNoEffect)
                {
                    // Frame events are called as messages are received from the server.
                    console() << "**INFO: Calling update() has no effect in the threaded client." << std::endl;
                    DidAlertAsyncNoEffect = true;
                }
            }
        }

        #pragma mark - Drawing

        void draw()
        {
            std::lock_guard<std::mutex> lock(mClientDataMutex);
            MPENonThreadedClient::draw();
        }
    };
}

MPEClientRef MPEClient::Create(MPEApp *app, bool isThreaded)
{
    if (isThreaded)
    {
        return MPEClientRef(new MPEThreadedClient(app));
    }
    else
    {
        return MPEClientRef(new MPENonThreadedClient(app));
    }
}

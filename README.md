#Most-Pixels-Ever for Cinder

A Cinder client for Most Pixels Ever; a library that synchronizes frame-based applications across multiple screens.

Read more about the project:
https://github.com/shiffman/Most-Pixels-Ever

###Example Usage:

```c++
class MyCinderApp : public AppNative
{
    public:      
    
    void        setup();    
    void        update();
    void        updateFrame(long serverFrameNumber);    
    void        draw();
    void        drawViewport(bool isNewFrame);    
    void        stringDataReceived(const std::string & message);
    
    private:
    
    MPEAsyncClient *mClient;      
    BouncyBall      mBall;
}

void MyCinderApp::setup()
{
    // Create the client with your settings file.
    // A sample settings.xml can be found in the assets folder.
    // Each client needs a unique settings file.        
    mClient = new MPEAsyncClient("settings.xml");
    
    // Set the event handlers.
    mClient->setFrameUpdateCallback(boost::bind(&MyCinderApp::updateFrame, this, _1));
    mClient->setDrawCallback(boost::bind(&MyCinderApp::drawViewport, this, _1));    
    mClient->setStringDataCallback(boost::bind(&MyCinderApp::stringDataReceived, this, _1));
}

void MyCinderApp::update()
{
    // Connect the client if we're not already.
    // Nothing else needs to happen in update().
    
    if (!mClient->isConnected() &&
        getElapsedFrames() % 60 == 0)
    {
        // Attempt to reconnect every 60 frames.
        mClient->start();
    }
}

void MyCinderApp::updateFrame(long serverFrameNumber)
{
    // This is where the app state should be modified. 
    // The FrameUpdateCallback is called whenever we get a message from the server,
    // which may be less frequent than update() or draw() is called.
    
    mBall.update();
}

void MyCinderApp::draw()
{
    mClient->draw();
}

void MyCinderApp::drawViewport(bool isNewFrame)
{
    // The viewport is automatically translated by the client so each
    // machine is only drawing a segment of the scene.
    gl::clear(Color(0,0,0));
    mBall.draw();
}

void MyCinderApp::stringDataReceived(const std::string & message)
{
    // Apps can broadcast data to the other clients. E.g.:
    //
    // mClient->sendStringData("mouse_pos: 100,200");
    // 
    
    ci::app::console() << "Received broadcast data: " << message << std::endl;
}
```
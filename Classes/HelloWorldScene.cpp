#include "HelloWorldScene.h"

USING_NS_CC;

#define VERBOSE_PERFORMANCE_TIME 1

static inline float getTimeDifferenceMS(const timeval& start, const timeval& end)
{
    return ((end.tv_sec - start.tv_sec)*1000.0f
              +(end.tv_usec - start.tv_usec) / 1000.0f);
}

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    this->addChild(sprite, 0);
    
    
    
    Size winSize = Director::getInstance()->getVisibleSize();
    bool portrait = winSize.width < winSize.height;
    float imageWidth = portrait ? winSize.width : winSize.height;
    ScanData::precomputeValues(imageWidth);
    pointsParent = Node::create();
    pointsParent->setPosition((portrait ? Vec2(0, (winSize.height - winSize.width) / 2) : Vec2((winSize.width - winSize.height) / 2, 0)) + Director::getInstance()->getVisibleOrigin());
    this->addChild(pointsParent);
    
#if VERBOSE_PERFORMANCE_TIME
    timeval startTime;
    gettimeofday(&startTime, NULL);
#endif
#if IMAGE == KIDNEY
    data.readFromFile("data_kydney.csv", true);
#elif IMAGE == TEST
    data.readFromFile("image.txt", true);
#else
    data.readFromFile("image.txt", true);
#endif
#if VERBOSE_PERFORMANCE_TIME
    timeval endTime;
    gettimeofday(&endTime, NULL);
    CCLOG("Data loaded in %f ms", getTimeDifferenceMS(startTime, endTime));
#endif
    Director::getInstance()->getScheduler()->schedule(CC_SCHEDULE_SELECTOR(HelloWorld::updatePoints), this, 0, CC_REPEAT_FOREVER, 0.5, false);
    
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

void HelloWorld::updatePoints(float dt) {
    generatePoints(data);
}

/* This is a performance-critical function, which has had several optimizations already :
 - minimize allocations : all allocations should happen on the first run (that's why most variables are static)
 - Sprites are reused, and never de-allocated
 - avoid re-calculating a variable that stay constant during a session: static variable with direct initialization are only calculated during first run
 - limit all function calls (for exemple, visibility is only set when there is a change)
 */
void HelloWorld::generatePoints(ScanData& data) {
#if VERBOSE_PERFORMANCE_TIME
    static timeval initTime;
    gettimeofday(&initTime, NULL);
#endif
    //Note: cocos2d-x should auto-batch draw calls together. If it's not good enough, use SpriteBatchNode
    //Texture is only loaded one time, so it is not a concern.
    //Note: the following variables are static to be calculated only once. However, they should be re-calculated if the visible size changes
    static Size winSize = Director::getInstance()->getVisibleSize();
    static bool portrait = winSize.width < winSize.height;
    static float imageWidth = portrait ? winSize.width : winSize.height;
    //Default sprite scale to be about 1 pixel
    static float spriteScale = DISPLAY_IMAGE_SIZE / imageWidth;
    static size_t lastVisible = 0;
#if VERBOSE_PERFORMANCE_TIME
    static timeval startTime;
    gettimeofday(&startTime, NULL);
#endif
    static std::vector<PointData> positions;
    //Compute positions
    positions = data.getPositions(imageWidth);
#if VERBOSE_PERFORMANCE_TIME
    static timeval intermediateTime;
    gettimeofday(&intermediateTime, NULL);
#endif
    static size_t i;
    static Sprite* currentPoint;
    //Re-use allocated sprites instead of allocating new ones. Here, we create missing points
    while (visualPoints.size() < positions.size()) {
#if DISPLAY_IMAGE == POINT
        currentPoint = Sprite::create("point.png");
#else
        currentPoint = Sprite::create("square.png");
#endif
        currentPoint->setScale(spriteScale * DISTANCE_INCREMENT * imageWidth);
        pointsParent->addChild(currentPoint);
        visualPoints.pushBack(currentPoint);
    }
    //Main loop, to assign points positions
    for(i = 0; i < positions.size(); i++) {
        visualPoints.at(i)->setScaleY(spriteScale * positions[i].distance);
        visualPoints.at(i)->setRotation(positions[i].angle);
        visualPoints.at(i)->setPosition(positions[i].position);
        visualPoints.at(i)->setOpacity(positions[i].opacity);
    }
    //Only set visible points that were not previously visible
    while (lastVisible < i) {
        visualPoints.at(lastVisible)->setVisible(true);
        lastVisible++;
    }
    if(lastVisible >= i) {
        lastVisible = i - 1;
    }
    //Hide remaining points (do NOT deallocate them, they may be useful for another image)
    while (i < visualPoints.size()) {
        visualPoints.at(i)->setVisible(false);
        i++;
    }
#if VERBOSE_PERFORMANCE_TIME
    static timeval endTime;
    gettimeofday(&endTime, NULL);
    CCLOG("Init in %f ms, points computed in %f ms, points updated in %f ms, total %f ms", getTimeDifferenceMS(initTime, startTime), getTimeDifferenceMS(startTime, intermediateTime), getTimeDifferenceMS(intermediateTime, endTime), getTimeDifferenceMS(initTime, endTime));
#endif
}

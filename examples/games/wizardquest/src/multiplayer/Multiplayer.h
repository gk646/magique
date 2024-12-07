#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

struct Multiplayer final
{
    static void init();
    static void update();
    static void postUpdate();

private:
    static void checkBeginSession();
};


#endif //MULTIPLAYER_H
#ifndef APPLICATION_H
#define APPLICATION_H


namespace magique
{
    struct Game
    {
        const char *const gameName;
        explicit Game(const char *name = "MyGame");
        virtual ~Game();

        virtual void init();
        virtual void update();
        virtual void render();
        virtual int run();
    };

} // namespace magique
#endif // APPLICATION_H
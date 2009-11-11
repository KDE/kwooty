#ifndef UNIQUEAPP_H
#define UNIQUEAPP_H

class MainWindow;
#include <KUniqueApplication>


class UniqueApp : public KUniqueApplication
{

public:
    UniqueApp();
    virtual ~UniqueApp();
    virtual int newInstance();

private:
    MainWindow* mainWindow;
    bool kwootyInstance;

};

#endif // UNIQUEAPP_H

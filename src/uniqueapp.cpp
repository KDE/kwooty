#include "uniqueapp.h"

#include "mainwindow.h"
#include <KIcon>
#include <KCmdLineArgs>
#include <KDebug>
#include <KUrl>

UniqueApp::UniqueApp() : KUniqueApplication()
{
    this->kwootyInstance = false;
}

UniqueApp::~UniqueApp()
{
}




int UniqueApp::newInstance()
{

    // create a new instance :
    if (!this->kwootyInstance) {

        this->kwootyInstance = true;

        this->setWindowIcon(KIcon("kwooty"));

        mainWindow = new MainWindow();
        mainWindow->show();
    }

    // instance already exists :
    if (this->kwootyInstance) {

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        // open nzb files set as argument :
        for (int i = 0; i < args->count(); i++) {

            this->mainWindow->openFileSilently(args->url(i));

        }

        args->clear();
    }

    return 0;
}

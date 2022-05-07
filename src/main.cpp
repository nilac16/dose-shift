#include "main-window.h"


wxIMPLEMENT_APP(MainApplication);


bool MainApplication::OnInit()
{
    try {
        MainWindow *mwnd = new MainWindow();
        mwnd->Show();
        return true;
    } catch (std::exception &) {
        return false;
    }
}

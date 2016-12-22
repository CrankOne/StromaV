#include "TROOT.h"
#include "TSystem.h"
#include "TRint.h"
#include "TApplication.h"
#include "TGClient.h"
#include "TGLabel.h"
#include "TGFrame.h"
#include "TGLayout.h"
#include "TGComboBox.h"
#include "TGTextView.h"
#include "TGTextEntry.h"
#include "TGTextEdit.h"
#include "TInterpreter.h"
#include "Getline.h"

#include "P348CommandWidgetPlugin.h"

#include <fstream>
#include <regex>

# include <iostream>  // XXX

# if 1

// @(#)root/gui:$Id$
// Author: Renat R. Dusaev   01/09/2016

//_____________________________________________________________________________
//
// P348CommandWidgetPlugin
//
// Class used to redirect command line input/output filtering ASCII control
// sequences. Based on implementation of TGCommandPlugin.[h|cxx] found in
// release version 6.04.
//_____________________________________________________________________________

ClassImp(P348CommandWidgetPlugin)

static const char _static_ASCIIEscSeqRemovingRxStr[] = R"((\x9B|\x1B\[)[0-?]*[ -\/]*[@-~])";
static const std::regex _static_ASCIIEscSeqRemovingRx(_static_ASCIIEscSeqRemovingRxStr);

//______________________________________________________________________________
P348CommandWidgetPlugin::P348CommandWidgetPlugin(const TGWindow *p, UInt_t w, UInt_t h) :
      TGMainFrame(p, w, h),
      fBufferHandle(this),
      fOStream(&fBufferHandle)
{
   // P348CommandWidgetPlugin Constructor.

   SetCleanup(kDeepCleanup);
   fHf = new TGHorizontalFrame(this, 100, 20);
   fComboCmd   = new TGComboBox(fHf, "", 1);
   fCommand    = fComboCmd->GetTextEntry();
   fCommandBuf = fCommand->GetBuffer();
   fComboCmd->Resize(200, fCommand->GetDefaultHeight());
   fHf->AddFrame(fComboCmd, new TGLayoutHints(kLHintsCenterY |
                 kLHintsRight | kLHintsExpandX, 5, 5, 1, 1));
   fHf->AddFrame(fLabel = new TGLabel(fHf, "Command (local):"),
                 new TGLayoutHints(kLHintsCenterY | kLHintsRight,
                 5, 5, 1, 1));
   AddFrame(fHf, new TGLayoutHints(kLHintsLeft | kLHintsTop |
            kLHintsExpandX, 3, 3, 3, 3));
   fCommand->Connect("ReturnPressed()", "P348CommandWidgetPlugin", this,
                     "HandleCommand()");
   fStatus = new TGTextView(this, 10, 100, 1);
   if (gClient->GetStyle() < 2) {
      Pixel_t pxl;
      gClient->GetColorByName("#a0a0a0", pxl);
      fStatus->SetSelectBack(pxl);
      fStatus->SetSelectFore(TGFrame::GetWhitePixel());
   }
   AddFrame(fStatus, new TGLayoutHints(kLHintsLeft | kLHintsTop |
            kLHintsExpandX | kLHintsExpandY, 3, 3, 3, 3));
   fPid = gSystem->GetPid();
   TString defhist(Form("%s/.root_hist", gSystem->UnixPathName(
                        gSystem->HomeDirectory())));
   FILE *lunin = fopen(defhist.Data(), "rt");
   if (lunin) {
      char histline[256];
      while (fgets(histline, 256, lunin)) {
         histline[strlen(histline)-1] = 0; // remove trailing "\n"
         fComboCmd->InsertEntry(histline, 0, -1);
      }
      fclose(lunin);
   }
   fTimer = new TTimer(this, 1000);
   fTimer->Reset();
   fTimer->TurnOn();
   MapSubwindows();
   Resize(GetDefaultSize());
   MapWindow();
}

//______________________________________________________________________________
P348CommandWidgetPlugin::~P348CommandWidgetPlugin()
{
   // Destructor.

   TString pathtmp = TString::Format("%s/command.%d.log",
                                     gSystem->TempDirectory(), fPid);
   gSystem->Unlink(pathtmp);
   delete fTimer;
   Cleanup();
}

//______________________________________________________________________________
void P348CommandWidgetPlugin::CheckRemote(const char * /*str*/)
{
   // Check if actual ROOT session is a remote one or a local one.
   Pixel_t pxl;
   TApplication *app = gROOT->GetApplication();
   if (!app->InheritsFrom("TRint"))
      return;
   TString sPrompt = ((TRint*)app)->GetPrompt();
   Int_t end = sPrompt.Index(":root [", 0);
   if (end > 0 && end != kNPOS) {
      // remote session
      sPrompt.Remove(end);
      gClient->GetColorByName("#ff0000", pxl);
      fLabel->SetTextColor(pxl);
      fLabel->SetText(Form("Command (%s):", sPrompt.Data()));
   }
   else {
      // local session
      gClient->GetColorByName("#000000", pxl);
      fLabel->SetTextColor(pxl);
      fLabel->SetText("Command (local):");
   }
   fHf->Layout();
}

//______________________________________________________________________________
Bool_t P348CommandWidgetPlugin::HandleTimer(TTimer *t)
{
   // Handle timer event.

   if (t != fTimer) return kTRUE;
   CheckRemote("");
   return kTRUE;
}

# endif

//______________________________________________________________________________
int P348CommandWidgetPlugin::StreamBufferHandle::sync()
{
    //std::cout << "XXX SYNCHRONIZE: " << this->str() << std::endl;  // XXX
    fPublisher->fSStream << this->str();
    fPublisher->SyncTextView();
    int rc = std::stringbuf::sync();
    this->str("");  // clear myself
    return rc;
}

//______________________________________________________________________________
void P348CommandWidgetPlugin::SyncTextView()
{
    fStatus->LoadBuffer( fSStream.str().c_str() );
    fStatus->ShowBottom();
}

//______________________________________________________________________________
void P348CommandWidgetPlugin::SetFont( FontStruct_t & fntStructRef )
{
    fStatus->SetFont( fntStructRef );
}

//______________________________________________________________________________
void P348CommandWidgetPlugin::OverrideTempDir( const std::string & dirPath ) {
    fTempDirOverriden = dirPath;
}

//______________________________________________________________________________
void P348CommandWidgetPlugin::HandleCommand()
{
    const char *string = fCommandBuf->GetString();
    if (strlen(string) > 1) {
        // form temporary file path
        TString sPrompt = "root []";
        TString pathtmp = TString::Format("%s/command.%d.log",
                (fTempDirOverriden.empty() ? gSystem->TempDirectory() : fTempDirOverriden.c_str()),
                fPid);
        TApplication *app = gROOT->GetApplication();
        if (app->InheritsFrom("TRint"))
            sPrompt = ((TRint*)gROOT->GetApplication())->GetPrompt();
        // Note: re-creates the file!
        FILE *lunout = fopen(pathtmp.Data(), "w+t");
        if (lunout) {
            fputs(Form("%s%s\n",sPrompt.Data(), string), lunout);
            fclose(lunout);
        }
        gSystem->RedirectOutput(pathtmp.Data(), "a");
        gApplication->SetBit(TApplication::kProcessRemotely);
        gROOT->ProcessLine(string);
        fComboCmd->InsertEntry(string, 0, -1);
        if (app->InheritsFrom("TRint"))
            Gl_histadd((char *)string);
        gSystem->RedirectOutput(0);

        // Now, acquire redirected textual output, strip ASCII sequences
        // and put it into outomatically synchronized output stream:
        {
            std::ifstream t(pathtmp.Data());
            const std::string orig((std::istreambuf_iterator<char>(t)),
                                    std::istreambuf_iterator<char>());
            std::string strippedString;
            // Filter out
            std::regex_replace(std::back_inserter(strippedString),
                       orig.begin(), orig.end(),
                       _static_ASCIIEscSeqRemovingRx, "" );
            GetStream() << strippedString;
            GetStream().flush();  // ?

            //std::cout << "XXX SENT: " << strippedString << std::endl;  // XXX
        }

        fStatus->ShowBottom();
        CheckRemote(string);
        fCommand->Clear();
    }
}

// Original HandleCommand() method source snippet
# if 0
//______________________________________________________________________________
void P348CommandWidgetPlugin::HandleCommand()
{
   // Handle command line from the "command" combo box.

   const char *string = fCommandBuf->GetString();
   if (strlen(string) > 1) {
      // form temporary file path
      TString sPrompt = "root []";
      TString pathtmp = TString::Format("%s/command.%d.log",
                                        gSystem->TempDirectory(), fPid);
      TApplication *app = gROOT->GetApplication();
      if (app->InheritsFrom("TRint"))
         sPrompt = ((TRint*)gROOT->GetApplication())->GetPrompt();
      FILE *lunout = fopen(pathtmp.Data(), "a+t");
      if (lunout) {
         fputs(Form("%s%s\n",sPrompt.Data(), string), lunout);
         fclose(lunout);
      }
      gSystem->RedirectOutput(pathtmp.Data(), "a");
      gApplication->SetBit(TApplication::kProcessRemotely);
      gROOT->ProcessLine(string);
      fComboCmd->InsertEntry(string, 0, -1);
      if (app->InheritsFrom("TRint"))
         Gl_histadd((char *)string);
      gSystem->RedirectOutput(0);
      fStatus->LoadFile(pathtmp.Data());
      fStatus->ShowBottom();
      CheckRemote(string);
      fCommand->Clear();
   }
}
# endif

// This ctr can be used if ROOT community will decide to make the
// HandleCommand() method virtual.
# if 0
P348CommandWidgetPlugin::P348CommandWidgetPlugin(const TGWindow *p, UInt_t w, UInt_t h) 
        : TGCommandPlugin( p, w, h ),
          fBufferHandle(this),
          fOStream( &fBufferHandle ) {}
# endif


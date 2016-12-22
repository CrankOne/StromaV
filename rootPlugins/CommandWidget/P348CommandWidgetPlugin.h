// @(#)root/gui:$Id$
// Author: Renat R. Dusaev   01/09/2016
/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ROOT_TGCommandWidgetPlugin
#define ROOT_TGCommandWidgetPlugin

#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif

# include <sstream>

# if 1
class TGLabel;
class TGComboBox;
class TGTextEntry;
class TGTextBuffer;
class TGTextView;
class TTimer;

class P348CommandWidgetPlugin : public TGMainFrame {
private:
    class StreamBufferHandle : public std::stringbuf {
    private:
        P348CommandWidgetPlugin * fPublisher;
    protected:
        virtual int sync() override;
    public:
        StreamBufferHandle( P348CommandWidgetPlugin * p ) : fPublisher(p) {}
    };
protected:
   Int_t              fPid;               // current process id
   TGHorizontalFrame *fHf;                // horizontal frame
   TGLabel           *fLabel;             // "command :" label
   TGComboBox        *fComboCmd;          // commands combobox
   TGTextEntry       *fCommand;           // command text entry widget
   TGTextBuffer      *fCommandBuf;        // command text buffer
   TGTextView        *fStatus;            // output capture view
   TTimer            *fTimer;             // for local/remote update

   StreamBufferHandle fBufferHandle;
   std::ostringstream fSStream;
   std::ostream fOStream;

   std::string fTempDirOverriden;

   void SyncTextView();
public:
   P348CommandWidgetPlugin(const TGWindow *p, UInt_t w, UInt_t h);
   virtual ~P348CommandWidgetPlugin();

   void CheckRemote(const char * /*str*/);
   void HandleCommand();

   virtual Bool_t HandleTimer(TTimer *t);

   /// Sets font struct of oputputting widget.
   virtual void SetFont( FontStruct_t & );
    
   /// Overrides default temp directory.
   virtual void OverrideTempDir( const std::string & );

   std::ostream & GetStream() { return fOStream; }

   friend class StreamBufferHandle;
   ClassDef(P348CommandWidgetPlugin, 0) // Command (I/O redirection) plugin for the ROOT Browsers
};
# else

#ifndef ROOT_TGCommandPlugin
#include "TGCommandPlugin.h"
#endif

class P348CommandWidgetPlugin : protected TGCommandPlugin {
private:
    class StreamBufferHandle : public std::stringbuf {
    private:
        P348CommandWidgetPlugin * fPublisher;
    protected:
        virtual int sync() override;
    public:
        StreamBufferHandle( P348CommandWidgetPlugin * p ) : fPublisher(p) {}
    };
protected:
    StreamBufferHandle fBufferHandle;
    std::ostringstream fSStream;
    std::ostream fOStream;

    std::string fTempDirOverriden;

    void _sync_textview();
public:
    P348CommandWidgetPlugin(const TGWindow *p, UInt_t w, UInt_t h);

    /// Sets font struct of oputputting widget.
    virtual void SetFont( FontStruct_t & );
    
    /// Overrides default temp directory.
    virtual void OverrideTempDir( const std::string & );

    std::ostream & stream() { return fOStream; }

    /// Re-implements command handling; uses RAM location for redirecte output and 
    /// strips ASCII control sequences from output string.
    virtual void HandleCommand();
    
    // This lines below won't be neccessary if HandleCommand()
    // will become virtual... {
    using TGCommandPlugin::CheckRemote;
    using TGCommandPlugin::TObject;
    using TGCommandPlugin::TQObject;
    using TGCommandPlugin::HandleTimer;
    using TGCommandPlugin::operator new;
    using TGCommandPlugin::operator delete;
    // }

    friend class StreamBufferHandle;

    ClassDef(P348CommandWidgetPlugin, 0) // Command (I/O redirection) plugin for the ROOT Browsers
};

# endif

# endif



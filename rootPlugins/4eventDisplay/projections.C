/**@file projections.C
 * @brief A CLING C-macro providing individual view for certain detector
 * in p348g4 event display.
 */

void
projections( TEveScene * originalScene ) {
    TEveWindowSlot      * slot = NULL;
    TEveWindowFrame     * frame = NULL;
    TEveViewer * v = NULL;
    TEveScene * scene = NULL;

    const char * detStr = originalScene->GetElementName();
    scene = gEve->SpawnNewScene( TString::Format("%s-standalone-scene", detStr) );
    // TODO: add scene to "Scenes-standalone"

    # if 0
    {   // TODO: how to avoid useless projection manager here to just add
        // axes on view?
        TEveProjectionManager * mng = new TEveProjectionManager(TEveProjection::kPT_RhoZ);
        // possible: kPT_RPhi, kPT_RhoZ, kPT_3D
        TEveProjectionAxes* axes = new TEveProjectionAxes(mng);
        for( TEveElement::List_i i = originalScene->BeginChildren();
            originalScene->EndChildren() != i; ++i ) {
            if( !mng->ImportElements( *i, scene ) ) {
                fprintf(stderr, "Warning: element %p of scene %p is not projectible.\n",
                        *i, originalScene );
                scene->AddElement( *i );
            }
        }
        //mng->ImportElements( scene );
        //mng->ImportElements( originalScene, scene );  // doesn't work as expected
        scene->AddElement( axes );
        scene->AddElement( mng );
    }
    # else
    for( TEveElement::List_i i = originalScene->BeginChildren();
         originalScene->EndChildren() != i; ++i ) {
        scene->AddElement( *i );
    }
    # endif

    // TODO: set tab name
    slot = TEveWindow::CreateWindowInTab(gEve->GetBrowser()->GetTabRight());
    TEveWindowPack* pack1 = slot->MakePack();
    pack1->SetShowTitleBar(kFALSE);
    pack1->SetHorizontal();

    pack1->SetName(  TString::Format("%s-dedicated", detStr) );
    pack1->SetTitle( TString::Format("Dedicated %s view", detStr) );

    // Embedded viewer.
    slot = pack1->NewSlot();
    v = new TEveViewer( TString::Format("%s-dedicated-viewer", detStr) );
    v->SpawnGLEmbeddedViewer(gEve->GetEditor());
    slot->ReplaceWindow(v);
    v->SetElementName("Front projection");
    v->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
    // TODO: add viewer to "Viewers-standalone"

    gEve->GetViewers()->AddElement(v);
    v->AddScene( scene );

    slot = pack1->NewSlot();
    TEveWindowPack* pack2 = slot->MakePack();
    pack2->SetShowTitleBar(kFALSE);

    slot = pack2->NewSlot();
    slot->StartEmbedding();
    TCanvas* can = new TCanvas(TString::Format("%s-dedicated-canvas", detStr));
    can->ToggleEditor();
    slot->StopEmbedding();

    // SA viewer.
    slot = pack2->NewSlot();
    v = new TEveViewer(TString::Format("%s-dedicated-aux-viewer", detStr));
    v->SpawnGLViewer(gEve->GetEditor());
    slot->ReplaceWindow(v);
    gEve->GetViewers()->AddElement(v);
    v->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOZ);
    v->AddScene( scene );
    // TODO: add viewer to "Viewers-standalone"
}


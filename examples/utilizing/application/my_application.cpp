/**@file my_application.cpp
 * @brief An example demonstrating somewhat generic application inheriting
 *        StromaV application mixins.
 *
 * This example demonstrates somewhat generic implementation of user
 * application utilizing the StromaV's application infrastructure.
 *
 * @ingroup app
 */

// This is for application routines:
# include "app/analysis.hpp"

class MyApp : //public sV::mixins::Geant4Application,  //< You may include
              //public sV::mixins::RootApplication     //< mixins here
              //# ifdef RPC_PROTOCOLS           //< Do not forget about
              //, public sV::mixins::PBEventApp //< feature-enabling macros
              //# endif 
              virtual public AbstractApplication
{
private:
    // ... your application attributes ...
protected:
    // Here one may override various StromaV methods to re-define particular
    // behaviour.
    // For example, to append common config with custom parameters:
    //![Appending common configuration]
    virtual void _V_concrete_app_append_common_cfg( const Config * appCfgPtr ) override {
        // you may immediately perform reading from application config here, or
        // defer it to _V_configure_concrete_app() / _V_run() depending on what
        // will be suitable for your goals:
        auto filePath = app_option<goo::filesystem::Path>("input-file");
        // ...
        _configuration.insertion_proxy()
            .p<std::string>( "myParameter1", "Some parameter." )
            .p<int>( "myParameter2", "Another parameter." )
            // ...
        ;
    }
    //![Appending common configuration]
    //![Setting up common configuration]
    virtual void _V_concrete_app_configure() override {
        // you may perform reading from application config here, or defer it to
        // _V_run() depending on what will be suitable for your goals.
        auto filePath = app_option<goo::filesystem::Path>("input-file");
        // ...
        // but the major purpose of this method is to operate with common config
        // parameters introduced at _V_concrete_app_append_common_cfg():
        std::string p1 = cfg_option<std::string>("myParameter1");
        int p2 = cfg_option<int>("myParameter2");
        // ...
    }
    //![Setting up common configuration]
    // ... other re-definition here
public:
    MyApp( Config * vm ) : AbstractApplication(vm) {
        // Note, that to inject additional parameter entries into the application
        // config, the shortes way is to append them in ctr like that:
        vm->insertion_proxy()
        .list<goo::filesystem::Path>('i', "input-file",
            "Input file --- an actual data source.").required_argument()? 
        ;
        // ...
    }
    // ...
};  // class MyApp


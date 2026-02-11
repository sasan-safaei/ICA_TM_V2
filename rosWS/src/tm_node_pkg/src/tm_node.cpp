#include "rclcpp/rclcpp.hpp"

#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/int32.hpp"
#include "../../../../TM_V2/Source/main_pi.h"

#include "my_msgs_pkg/msg/my_msg_tm_rsu_pub.hpp"
#include "my_msgs_pkg/msg/my_msg_tm_sts_pub.hpp"
#include "my_msgs_pkg/msg/my_msg_tm_msg_box_pub.hpp"
#include "my_msgs_pkg/msg/my_msg_qt_pub.hpp"

#include <vector>

class TMNode : public rclcpp::Node {
public:
    
    TMNode() : Node("tm_node") {
        // Publisher
        //const char* args[] = { "programName", "-workspace", "/home/ica/ica_usv_test_resualt/"};
        //int argc = sizeof(args) / sizeof(args[0]);
        //char* argv[argc];
        //for (int i = 0; i < argc; ++i) {
        //argv[i] = const_cast<char*>(args[i]);}
        //myApp.initialize(argc,argv);
        
        const char* args[] = {
            "programName",
            "-workspace",
            "/home/ica/ica_usv_test_result/"
        };

        int argc = sizeof(args) / sizeof(args[0]);

        // Create a vector of char* pointers
        std::vector<char*> argv(argc);
        for (int i = 0; i < argc; ++i) {
            argv[i] = const_cast<char*>(args[i]);
        }

        // Pass C-style pointers to your app
        myApp.initialize(argc, argv.data());

        tm_run_test_result = this->create_publisher<my_msgs_pkg::msg::MyMsgTmRsuPub>("tm_test_result",10);        
        tm_run_msgbox = this->create_publisher<my_msgs_pkg::msg::MyMsgTmMsgBoxPub>("tm_msg_box",10);                    
        tm_run_status = this->create_publisher<my_msgs_pkg::msg::MyMsgTmStsPub>("tm_status_result",10);
    
        
        timer_ = this->create_wall_timer( std::chrono::milliseconds(20), std::bind(&TMNode::timer_callback, this));        
        // Subscriber
        qt_subscriber_ = this->create_subscription<my_msgs_pkg::msg::MyMsgQtPub>( "qt_pub", 10, std::bind(&TMNode::qt_callback, this, std::placeholders::_1) );
        
        RCLCPP_INFO(this->get_logger(), "TMNode is running.");
    }

private:
    my_msgs_pkg::msg::MyMsgTmRsuPub tm_res_msg;
    my_msgs_pkg::msg::MyMsgTmMsgBoxPub tm_msg_box_msg;
    my_msgs_pkg::msg::MyMsgTmStsPub tm_sts_msg;
    _interact_registers myLastInterActReg;
    
    rclcpp::Subscription<my_msgs_pkg::msg::MyMsgQtPub>::SharedPtr qt_subscriber_;
    rclcpp::Publisher<my_msgs_pkg::msg::MyMsgTmRsuPub>::SharedPtr tm_run_test_result;    
    rclcpp::Publisher<my_msgs_pkg::msg::MyMsgTmMsgBoxPub>::SharedPtr tm_run_msgbox;
    
    rclcpp::Publisher<my_msgs_pkg::msg::MyMsgTmStsPub>::SharedPtr tm_run_status;

    rclcpp::TimerBase::SharedPtr timer_;

    void timer_callback() {
        static uint32_t sec_20ms;
        if(sec_20ms++>100){//eash 2 Sec            
            //if(tm_sts_msg.tm_run_sts!=2){//in Main Menu *****                
                std::string reply = myApp.send("hello");        
                reply.erase(std::remove(reply.begin(), reply.end(), '\r'), reply.end());
                reply.erase(std::remove(reply.begin(), reply.end(), '\n'), reply.end());
//use for debug //RCLCPP_INFO(this->get_logger(), "AppR: '%s' sts(%d) dongle: %s", reply.c_str(), myInterActReg.running_status,myInterActReg.Dongle.c_str() );
                myApp.pre_Check();        
            //}
            sec_20ms=0;
//use for debug //RCLCPP_INFO(this->get_logger(), "Current InterActReg: [%s]", myLastInterActReg.TR.show_str().c_str());                
        }
        
        std::string csLogTemp=myInterActReg.csLogRead();
        tm_res_msg.ar_on=myLastInterActReg.TR.AR_On;
        tm_res_msg.ar_off=myLastInterActReg.TR.AR_Off;
        tm_res_msg.uart_con=myLastInterActReg.TR.UART_Con;
        tm_res_msg.vvcc=myLastInterActReg.TR.Vvcc;
        tm_res_msg.vcap_bank=myLastInterActReg.TR.VcapBank;
        tm_res_msg.vcap1=myLastInterActReg.TR.Vcap1;
        tm_res_msg.vcap2=myLastInterActReg.TR.Vcap2;
        tm_res_msg.vcap3=myLastInterActReg.TR.Vcap3;
        tm_res_msg.vcap4=myLastInterActReg.TR.Vcap4;
        tm_res_msg.cap_charge_time=myLastInterActReg.TR.CapChargeTime;
        tm_res_msg.vin=myLastInterActReg.TR.Vin;
        tm_res_msg.vout=myLastInterActReg.TR.VOut;
        tm_res_msg.in_current=myLastInterActReg.TR.InCurrent;
        tm_res_msg.load_current=myLastInterActReg.TR.LoadCurrent;
        tm_res_msg.current_test_no=myLastInterActReg.TR.currentTestNo;
        tm_res_msg.flyback_dis=myLastInterActReg.TR.FlyBackDis;
        tm_res_msg.flyback_en=myLastInterActReg.TR.FlayBackEn;
        tm_res_msg.out_sw_off=myLastInterActReg.TR.OutSwOff;
        tm_res_msg.out_sw_off_time=myLastInterActReg.TR.OutSWOffTime;
        tm_res_msg.cap_discharge_time=myLastInterActReg.TR.CapDisChargeTime;

        if(myInterActReg.msgBox.counter>0)
            if(myInterActReg.msgBox.counter!=myLastInterActReg.msgBox.counter ){
                tm_msg_box_msg.tm_msg_counter= myInterActReg.msgBox.counter;  
                tm_msg_box_msg.tm_msg_show_time= myInterActReg.msgBox.timeOutCnt;  
                tm_msg_box_msg.tm_msg= myInterActReg.msgBox.msg;  
                tm_msg_box_msg.tm_btn_yes= myInterActReg.msgBox.btnYesTxt;  
                tm_msg_box_msg.tm_btn_no= myInterActReg.msgBox.btnNoTxt;                                      
                tm_run_msgbox->publish(tm_msg_box_msg);
            }
       
        
        tm_res_msg.uid_str=myLastInterActReg.TR.UID_str;

        tm_run_test_result->publish(tm_res_msg);

    
        //if(csLogTemp!="" ){tm_sts_msg.tm_log=csLogTemp; tm_run_log_publisher_->publish(tm_sts_msg);}
        //if(myLastInterActReg.Dongle!=myInterActReg.Dongle){log_msg.data=myInterActReg.Dongle; tm_run_Dongle_->publish(log_msg);}
        tm_sts_msg.tm_log=csLogTemp;
        tm_sts_msg.tm_dongle=myInterActReg.Dongle.c_str();
        tm_run_status->publish(tm_sts_msg);
        
        
        myLastInterActReg=myInterActReg;
        
        
    }
    
    void qt_callback(const my_msgs_pkg::msg::MyMsgQtPub::SharedPtr msg) {
        printf("qtCallBack: Dongle:%d btn:%d\n",msg->dongle_sel,msg->btn_press);
        
        //tm_sts_msg.tm_run_sts = 0;
        tm_sts_msg.tm_log="";        
        if (msg->btn_press > 0) {
            switch (msg->btn_press) {
            case 1:
                if(msg->dongle_sel>0){
                    tm_sts_msg.tm_log = "gui Start button pressed!";
                    tm_sts_msg.tm_run_sts = 2;
                    if (msg->dongle_sel==5 || msg->dongle_sel==6)
                        tm_sts_msg.tm_run_sts = 3;
                    myInterActReg.gui_CMD = msg->dongle_sel;
                    myInterActReg.board_version = msg->board_version;
                    myApp.run();
                    
                }else{
                    tm_sts_msg.tm_log = "No Selected Dongle!!!";
                    myInterActReg.gui_CMD = 0;

                }
                
                break;
            case 2:
                tm_sts_msg.tm_log = "gui Stop button pressed!";                
                tm_sts_msg.tm_run_sts = 1;
                myInterActReg.gui_CMD = 0;  
                printf("Stop the myApp ....\n");
                myApp.stop();
                printf("Stop ok.\n");

                break;
            case 3:
            if(msg->dongle_sel>0){
                    myApp.stop();
                    tm_sts_msg.tm_log = "gui ReRun button pressed!";
                    tm_sts_msg.tm_run_sts = 2;
                    if (msg->dongle_sel==5 || msg->dongle_sel==6)
                        tm_sts_msg.tm_run_sts = 3;
                    myInterActReg.gui_CMD = msg->dongle_sel;
                    myInterActReg.board_version = msg->board_version;
                    myApp.run();
                    
                }else{
                    tm_sts_msg.tm_log = "No Selected Dongle!!!";
                    myInterActReg.gui_CMD = 0;                    

                }                
            break;
            default:
                tm_sts_msg.tm_log = "gui Unknown button pressed!";
                //tm_sts_msg.tm_log_cnt+=1;
                tm_sts_msg.tm_run_sts = 1;
                myApp.stop();
            }

            tm_run_status->publish(tm_sts_msg);
            RCLCPP_INFO(this->get_logger(),
                "Published log: '%s' + run_sts '%d'",
                tm_sts_msg.tm_log.c_str(),
                tm_sts_msg.tm_run_sts);
        }
        if(msg->msgbox_press>0)
            myInterActReg.msgBox.resualt=msg->msgbox_press;
        myInterActReg.DongleID = msg->dongle_sel;
        myInterActReg.board_version = msg->board_version;
        RCLCPP_INFO(this->get_logger(), "new Dongle Value '%d'", myInterActReg.DongleID);
        myApp.pre_Check(); 
        tm_run_status->publish(tm_sts_msg);
    }


};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TMNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
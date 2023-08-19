#include"test.pb.h"
#include<iostream>

int main(){
    ik::LoginResponse login_rsp;
    ik::ErrorMsg* error_msg = login_rsp.mutable_error();
    error_msg->set_error(1);
    error_msg->set_error_msg("login error");

    ik::GetFriendListResponse friend_rsp;
    ik::ErrorMsg* friend_msg = friend_rsp.mutable_error();
    friend_msg->set_error(1);
    friend_msg->set_error_msg("get friend list error");

    ik::User* user1 = friend_rsp.add_friendlists();
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(ik::User::man);

    std::cout<<"has friends: "<<friend_rsp.friendlists_size()<<std::endl;
    return 0;
}
#pragma once
#include <iostream>
#include "command_creator.h"
#include "command_executor.h"

const size_t TEST_N = 7;

int main()
{
    command_executor executor;

    size_t i = 0;
    std::string input[TEST_N];
    input[0] = "CreateTable Sample (ID:Int, Name:String, Value:Int) Index ON ID";
    input[1] = "CreateTable Hohoho (Name:String, FavFood:String, Birthday:Date DEFAULT 2002-01-24)";
    input[2] = "DropTable Sample";
    input[3] = "CreateTable HappyBirthday (HappinessLevel:Int, Name:String, Friends:Int, PartyAddress:String DEFAULT \"At home\")";
    input[4] = "CreateTable ThisOneHasToGo (SomeDate:DateTime)";
    input[5] = "CreateTable NiceShows (ShowId:Int, EpisodesCount:Int, HowMuchILikedI:Int) Index ON ShowId";
    input[6] = "DropTable ThisOneHasToGo";
    while (1) {
        std::cout << "FmiSql> ";
        try {
            std::getline(std::cin, input[i]);
        }
        catch (std::exception& e) {
            continue;
        }
        std::cout << "\n";
        
        if (input[i] == "Quit") {
            std::cout << "Goodbye\n";
            //executor.wrapitup();
            break;
        }

        command com;
        try {
            com = command_creator::com_from_line(input[i]);
        }
        catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
            continue;
        }

        try {
            executor.execute(com);
        }
        catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        i++;
        //if (i == TESTS_N) break;
    }
}

#include "include.hh"
#include "oauth.h"
#include <stdlib.h>     //for using the function sleep

#include <vector>

using json = nlohmann::json;
using std::vector;

namespace asio = boost::asio;
namespace dpp = discordpp;

std::string getToken();

std::istream &safeGetline(std::istream &is, std::string &t);

void readIt(bool er, json msg);

void addToList(std::string ruser, std::string interaction_id, std::string interaction_token, std::string dest, std::string item1, std::string item2, 
               std::string item3, std::string item4, std::string item5);
void addToList(std::string ruser, std::string interaction_id, std::string interaction_token, std::string dest, std::string item1, std::string item2, 
               std::string item3, std::string item4);
void addToList(std::string ruser, std::string interaction_id, std::string interaction_token, std::string dest, std::string item1, std::string item2, 
               std::string item3);
void addToList(std::string ruser, std::string interaction_id, std::string interaction_token, std::string dest, std::string item1, std::string item2);
void addToList(std::string ruser, std::string interaction_id, std::string interaction_token, std::string dest, std::string item1);

void filter(std::string &target, const std::string &pattern);

struct logiRequest {
    std::string ruser;

    std::string destination;

    std::string item1;
    std::string item2;
    std::string item3;
    std::string item4;
    std::string item5;

    std::string duser;
    bool accept = false;
    bool done = false;

    //Message deleteStuff
    std::string interaction_id;
    std::string interaction_token;
    std::string comp_id;
    };

    //Create Logi List (GLOBAL)
    vector<logiRequest> logiList;

//Map Locations
//https://shard2.foxholestats.com/images/worldmap2.png

int main() {
    dpp::log::filter = dpp::log::info;
    dpp::log::out = &std::cerr;

    std::cout << "Revving Engine..\n\n";

    std::string token = getToken();
    if (token.empty()) {
        std::cerr << "CRITICAL: "
                  << "There is no valid way for Echo to obtain a token! Use "
                     "one of the following ways:"
                  << std::endl
                  << "(1) Fill the BOT_TOKEN environment variable with the "
                     "token (e.g. 'Bot 123456abcdef')."
                  << std::endl
                  << "(2) Copy the example `token.eg.dat` as `token.dat` and "
                     "write your own token to it.\n";
        exit(1);
    }

    //Create bot object
    auto bot = std::make_shared<DppBot>();

    // Don't complain about unhandled events
    bot->debugUnhandled = false;

    // Declare the intent to receive guild messages
    // You don't need `NONE` it's just to show you how to declare multiple
    bot->intents = dpp::intents::NONE | dpp::intents::GUILD_MESSAGES;

    /*/
     * Create handler for the READY payload, this may be handled by the bot in
    the future.
     * The `self` object contains all information about the 'bot' user.
    /*/
    json self;
    bot->handlers.insert(
        {"READY",[&self, &bot](json data) {
             self = data["user"]; 
             
              json requestCommand = {
                {"name", "request"}, 
                {"description", "Request Logistics"}, 
                {"options",{ 
                    { 
                       {"name", "dest"}, 
                       {"description", "Destination for logi"}, 
                       {"type", 3}, 
                       {"required", true},
                    },
                    {
                        {"name", "item1"},
                        {"description", "Item you want"},
                        {"type", 3},
                        {"required", true},
                    },
                    {
                        {"name", "item2"},
                        {"description", "Item you want"},
                        {"type", 3},
                        {"required", false},
                    },
                    {
                        {"name", "item3"},
                        {"description", "Item you want"},
                        {"type", 3},
                        {"required", false},
                    },
                    {
                        {"name", "item4"},
                        {"description", "Item you want"},
                        {"type", 3},
                        {"required", false},
                    },
                    {
                        {"name", "item5"},
                        {"description", "Item you want"},
                        {"type", 3},
                        {"required", false}
                    }
                  }
                }
                };  

                /*
                bot->call()
                   ->method("DELETE")
                   ->target("/applications/873355455806730281/commands/874147308198064208")
                   ->onRead(readIt)
                   ->run();
                */

                //Submitting a Slash Command
                bot->callJson()
                    ->method("POST")
                    ->target("/applications/873355455806730281/guilds/873367900784832522/commands")
                    ->payload(requestCommand)
                    ->run();

                /*bot->callJson()
                    ->method("POST")
                    ->target("/applications/873355455806730281/guilds/866495914913824768/commands")
                    ->payload(requestCommand)
                    ->run();
                */
             }});

    bot->prefix = "~";
    

    //Handles incoming messages that the bot can see
        //
        bot->handlers.insert({"MESSAGE_CREATE", [&bot, &self](json msg){

            int counter = 0;

            //std::cout << msg.dump(4);

            //Determine if its interaction reaction
            if(msg["components"][0]["components"] != nlohmann::detail::value_t::null)
            {
                //std::cout << "Handler Incoming Message: " << msg.dump(4) << "\n\n";
                
                for(vector<logiRequest>::iterator it = logiList.begin(); it != logiList.end(); it++) {
                    std::string Comp_id = msg["id"].get<std::string>();
                        
                        
                        if(it->accept == false && it->comp_id == "")
                        {
                            //std::cout << "ACCEPT NEW HANDLES";
                            int queue = logiList.size() - 1;
                            logiList[queue].comp_id = Comp_id;
                        }
                        
                        if(it->accept == true)
                        {
                            //std::cout << "ACCEPT HANDLES";
                            logiList[counter].comp_id = Comp_id;
                        }
                    counter++;
                }
            }
        }});

    //Slash Handler 
    bot->handlers.insert({"INTERACTION_CREATE", [&bot](json msg){

        //std::cout << "SLASH HANDLER\n" << msg.dump(4);
        
        //Handles Buttons 
        if(msg["data"]["custom_id"] != nlohmann::detail::value_t::null){
        
        //Counter to determine which array position it is in. 
        int counter = 0;

            for(vector<logiRequest>::const_iterator it = logiList.begin(); it != logiList.end(); it++) {
                
                //Handles Deliverd button
                if (it->interaction_id + "/delivered" == msg["data"]["custom_id"].get<std::string>()){
                    std::string s_interaction_id = msg["id"].get<std::string>();
                    std::string interaction_token = msg["token"].get<std::string>();
                    std::string s_interaction_token = it->interaction_token;
                    std::string comp_id = it->comp_id;
                    
                    if(it->duser == msg["member"]["user"]["username"].get<std::string>())
                    {
                         bot->callJson()
                            ->method("POST")
                            ->target("/interactions/" + s_interaction_id + "/" + interaction_token + "/callback")
                            ->payload(json({{"type", 4},{"data", {{"content", "Thank you " + it->duser + " for your contribution.\nGodspeed WDF."}}}}))
                            ->run();

                        //Delete The buttons
                        bot->deleteMessage()
                           ->channel_id(dpp::get_snowflake(msg["channel_id"]))
                           ->message_id(dpp::get_snowflake(comp_id))
                           ->run();

                        //Delete The embeds
                        bot->call()
                           ->method("DELETE")
                           ->target("/webhooks/873355455806730281/" + s_interaction_token + "/messages/@original")
                           ->run();
                    
                        logiList[counter] = logiList.back();
                        logiList.pop_back();
                        break;

                    } else {
                        bot->callJson()
                           ->method("POST")
                           ->target("/interactions/" + s_interaction_id + "/" + interaction_token + "/callback")
                           ->payload(json({{"type", 1}}))
                           ->run();

                        break;
                    }

                }

                //Handles Accept Button
                if (it->interaction_id == msg["data"]["custom_id"].get<std::string>()) {
                    
                    //std::cout << "Handler Accpet Button: " << msg.dump(4) << "\n\n";

                    std::string channel_id = msg["channel_id"].get<std::string>();
                    std::string rUser = msg["member"]["user"]["username"].get<std::string>();
                    std::string s_interaction_id = msg["id"].get<std::string>();
                    std::string interaction_token = msg["token"].get<std::string>();
                    std::string s_interaction_token = it->interaction_token;
                    std::string dest = it->destination;
                    std::string comp_id = it->comp_id;

                    json embeds;

                    //Delete The embeds
                    bot->call()
                       ->method("DELETE")
                       ->target("/webhooks/873355455806730281/" + s_interaction_token + "/messages/@original")
                       ->run();

                    //Delete The buttons
                    bot->deleteMessage()
                        ->channel_id(dpp::get_snowflake(msg["channel_id"]))
                        ->message_id(dpp::get_snowflake(comp_id))
                        ->run();

                    //Determine how many items there are
                        if(it->item5 != "")
                        {
                            std::string item1 = it->item1;
                            std::string item2 = it->item2;
                            std::string item3 = it->item3;
                            std::string item4 = it->item4;
                            std::string item5 = it->item5;

                            embeds = 
                            {{
                                {"title", "LogiRequest     \n\nDestination: " + dest + "\n\nItem's List :"},
                                {"description", "\n     " + it->item1 + "     \n" + it->item2 + "\n     " + it->item3 + "\n     " + it->item4 + "\n     " + it->item5},
                                {"color", "4718336"}
                            }};
                        } else if(it->item4 != "")
                        {
                            std::string item1 = it->item1;
                            std::string item2 = it->item2;
                            std::string item3 = it->item3;
                            std::string item4 = it->item4;

                            embeds = 
                            {{
                                {"title", "LogiRequest     \n\nDestination: " + dest + "\n\nItem's List :"},
                                {"description", "\n     " + it->item1 + "     \n" + it->item2 + "\n     " + it->item3 + "\n     " + it->item4},
                                {"color", "4718336"}
                            }};
                        } else if(it->item3 != "")
                        {
                            std::string item1 = it->item1;
                            std::string item2 = it->item2;
                            std::string item3 = it->item3;
                            embeds = 
                            {{
                                {"title", "LogiRequest     \n\nDestination: " + dest + "\n\nItem's List :"},
                                {"description", "\n     " + it->item1 + "     \n" + it->item2 + "\n     " + it->item3 },
                                {"color", "4718336"}
                            }};
                        } else if(it->item2 != "")
                        {
                            std::string item1 = it->item1;
                            std::string item2 = it->item2;

                            embeds = 
                            {{
                                {"title", "LogiRequest     \n\nDestination: " + dest + "\n\nItem's List :"},
                                {"description", "\n     " + it->item1 + "     \n" + it->item2},
                                {"color", "4718336"}
                            }};
                        } else if(it->item1 != "")
                        {
                            std::string item1 = it->item1;

                            embeds = 
                            {{
                                {"title", "LogiRequest     \n\nDestination: " + dest + "\n\nItem's List :"},
                                {"description", "\n    " + it->item1},
                                {"color", "4718336"}
                            }};   
                        }



                    logiList[counter].accept = true;
                    logiList[counter].duser = rUser;

                    //std::cout << "\n" << logiList[counter].accept << "\n";

                    json comp = 
                    {
                        {"content", "Accepted by: " + rUser},
                        {"components", {
                            {
                                {"type", 1},
                                {"components", 
                                    {{
                                        {"type", 2},
                                        {"label", "CANCEL LOGI"},
                                        {"style", 4},
                                        {"custom_id", s_interaction_id + "/cancel"},
                                    
                                    },
                                    {
                                        {"type", 2},
                                        {"label", "Delivered"},
                                        {"style", 1},
                                        {"custom_id", s_interaction_id + "/delivered"},
                                    }}
                                }
                            } 
                          }
                        }
                    };

                    //Create Embed for confirmation 
                    bot->callJson()
                       ->method("POST")
                       ->target("/interactions/" + s_interaction_id + "/" + interaction_token + "/callback")
                       ->payload(json({{"type", 4},{"data", {{"embeds", embeds}}}}))
                       ->run();

                    bot->callJson()
                       ->method("POST")
                       ->target("/channels/" + channel_id + "/messages")
                       ->payload(comp)
                       ->run();

                    logiList[counter].interaction_token = interaction_token;
                    logiList[counter].interaction_id = s_interaction_id;

                }

                 //Handles Canceling and deletion of the request
                if (it->interaction_id + "/cancel" == msg["data"]["custom_id"].get<std::string>()){
                    std::string s_interaction_id = msg["id"].get<std::string>();
                    std::string interaction_token = msg["token"].get<std::string>();
                    std::string s_interaction_token = it->interaction_token;
                    std::string comp_id = it->comp_id;

                    if(it->ruser != msg["member"]["user"]["username"].get<std::string>() || it->duser != msg["member"]["user"]["username"].get<std::string>())
                    {
                        bot->callJson()
                           ->method("POST")
                           ->target("/interactions/" + s_interaction_id + "/" + interaction_token + "/callback")
                           ->payload(json({{"type", 1}}))
                           ->run();

                        break;
                    }

                    bot->callJson()
                       ->method("POST")
                       ->target("/interactions/" + s_interaction_id + "/" + interaction_token + "/callback")
                       ->payload(json({{"type", 1}}))
                       ->run();

                    //Delete The buttons
                    bot->deleteMessage()
                        ->channel_id(dpp::get_snowflake(msg["channel_id"]))
                        ->message_id(dpp::get_snowflake(comp_id))
                        ->run();

                    //Delete The embeds
                    bot->call()
                       ->method("DELETE")
                       ->target("/webhooks/873355455806730281/" + s_interaction_token + "/messages/@original")
                       ->run();
                    
                    logiList[counter] = logiList.back();
                    logiList.pop_back();
                    break;
                    //counter--;
                }
                counter++;
            }
        }

        //Handle regular interaction requests
        if(msg["data"]["name"] != nlohmann::detail::value_t::null){
        if (msg["data"]["name"].get<std::string>() == "request")
        {
            //std::cout << msg.dump(4);

            std::string channel_id = msg["channel_id"].get<std::string>();
            std::string s_interaction_id = msg["id"].get<std::string>();
            std::string interaction_token = msg["token"].get<std::string>();
            //std::string message = msg["data"]["options"][0]["value"].get<std::string>();
            std::string rUser = msg["member"]["user"]["username"].get<std::string>();
            std::string dest = msg["data"]["options"][0]["value"].get<std::string>();
            std::string item1 = msg["data"]["options"][1]["value"].get<std::string>();
            json embeds;

            if(msg["data"]["options"][5]["value"] != nlohmann::detail::value_t::null)
            {
                std::string item2 = msg["data"]["options"][2]["value"].get<std::string>();
                std::string item3 = msg["data"]["options"][3]["value"].get<std::string>();
                std::string item4 = msg["data"]["options"][4]["value"].get<std::string>();
                std::string item5 = msg["data"]["options"][5]["value"].get<std::string>();
                
                //{rUser, Interaction_token, destination, item1, item2, item3, item4, item5}
                addToList(rUser, s_interaction_id, interaction_token, dest, item1, item2, item3, item4, item5);

                embeds = 
                {{
                    {"title", "LogiRequest \n\n Item's List :"},
                    {"description", item1 + "\n     " + item2 + "\n" + item3 + "\n" + item4 + "\n" + item5},
                    {"color", "5814783"}
                }};
            }
            else if(msg["data"]["options"][4]["value"] != nlohmann::detail::value_t::null)
            {
                std::string item2 = msg["data"]["options"][2]["value"].get<std::string>();
                std::string item3 = msg["data"]["options"][3]["value"].get<std::string>();
                std::string item4 = msg["data"]["options"][4]["value"].get<std::string>();

                //{rUser, s_interaction_id, destination, item1, item2, item3, item4, item5}
                addToList(rUser, s_interaction_id, interaction_token, dest, item1, item2, item3, item4);

                embeds = 
                {{
                    {"title", "LogiRequest \n\n Item's List :"},
                    {"description", item1 + "\n" + item2 + "\n" + item3 + "\n" + item4},
                    {"color", "5814783"}
                }};
            }
            else if(msg["data"]["options"][3]["value"] != nlohmann::detail::value_t::null)
            {
                std::string item2 = msg["data"]["options"][2]["value"].get<std::string>();
                std::string item3 = msg["data"]["options"][3]["value"].get<std::string>();

                //{rUser, s_interaction_id, destination, item1, item2, item3, item4, item5}
                addToList(rUser, s_interaction_id, interaction_token, dest, item1, item2, item3);

                embeds = 
                {{
                    {"title", "LogiRequest \n\n Item's List :"},
                    {"description", item1 + "\n" + item2 + "\n" + item3},
                    {"color", "5814783"}
                }};
            }
            else if(msg["data"]["options"][2]["value"] != nlohmann::detail::value_t::null)
            {
                std::string item2 = msg["data"]["options"][2]["value"].get<std::string>();

                //{rUser, s_interaction_id, destination, item1, item2, item3, item4, item5}
                addToList(rUser, s_interaction_id, interaction_token, dest, item1, item2);
                embeds = 
                {{
                    {"title", "LogiRequest \n\n Item's List :"},
                    {"description", item1 + "\n" + item2},
                    {"color", "5814783"}
                }};
            }
            else if(msg["data"]["options"][1]["value"] != nlohmann::detail::value_t::null)
            {

                //{rUser, s_interaction_id, destination, item1, item2, item3, item4, item5}
                addToList(rUser, s_interaction_id, interaction_token, dest, item1);

                embeds = 
                {{
                    {"title", "LogiRequest     \n\nDestination: " + dest + "\n\nItem's List :"},
                    {"description", item1},
                    {"color", "5814783"}
                }};
            }

            json comp = 
            {
                {"content", "Requested by: " + rUser},
                {"components", {
                    {
                        {"type", 1},
                        {"components", 
                            {{
                                {"type", 2},
                                {"label", "ACCEPT"},
                                {"style", 1},
                                {"custom_id", s_interaction_id},
                            }}
                        }
                    } 
                  }
                }
            };

            bot->callJson()
              ->method("POST")
              ->target("/interactions/" + s_interaction_id + "/" + interaction_token + "/callback")
              ->payload(json({{"type", 4},{"data", {{"embeds", embeds}}}}))
              ->run();

            bot->callJson()
            ->method("POST")
            ->target("/channels/" + channel_id + "/messages")
            ->payload(comp)
            ->run();   
        
        } }


        }});

    // Create Asio context, this handles async stuff.
    auto aioc = std::make_shared<asio::io_context>();

    // Set the bot up
    bot->initBot(6, token, aioc);

    // Run the bot!
    bot->run();

    return 0;
}

//Helper function to read out any Get
void readIt(bool er, json msg)
{
    std::cout << msg.dump(4);
}

void addToList(std::string ruser, std::string interaction_id, std::string interaction_token, std::string dest, std::string item1, std::string item2, 
               std::string item3, std::string item4, std::string item5)
{
    logiList.push_back(logiRequest());
    //Get our queue number
    int queue = logiList.size() - 1;
    
    logiList[queue].ruser = ruser;
    logiList[queue].interaction_id = interaction_id;
    logiList[queue].interaction_token = interaction_token;
    logiList[queue].destination = dest;
    logiList[queue].item1 = item1;
    logiList[queue].item2 = item2;
    logiList[queue].item3 = item3;
    logiList[queue].item4 = item4;
    logiList[queue].item5 = item5;


}

void addToList(std::string ruser, std::string interaction_id, std::string interaction_token, std::string dest, std::string item1, std::string item2, 
               std::string item3, std::string item4)
{
    logiList.push_back(logiRequest());
    //Get our queue number
    int queue = logiList.size() - 1;
    
    logiList[queue].ruser = ruser;
    logiList[queue].interaction_id = interaction_id;
    logiList[queue].interaction_token = interaction_token;
    logiList[queue].destination = dest;
    logiList[queue].item1 = item1;
    logiList[queue].item2 = item2;
    logiList[queue].item3 = item3;
    logiList[queue].item4 = item4;


}

void addToList(std::string ruser, std::string interaction_id, std::string interaction_token, std::string dest, std::string item1, std::string item2, 
               std::string item3)
{
    logiList.push_back(logiRequest());
    //Get our queue number
    int queue = logiList.size() - 1;
    
    logiList[queue].ruser = ruser;
    logiList[queue].interaction_id = interaction_id;
    logiList[queue].interaction_token = interaction_token;
    logiList[queue].destination = dest;
    logiList[queue].item1 = item1;
    logiList[queue].item2 = item2;
    logiList[queue].item3 = item3;



}

void addToList(std::string ruser, std::string interaction_id, std::string interaction_token, std::string dest, std::string item1, std::string item2)
{
    logiList.push_back(logiRequest());
    //Get our queue number
    int queue = logiList.size() - 1;
    
    logiList[queue].ruser = ruser;
    logiList[queue].interaction_id = interaction_id;
    logiList[queue].interaction_token = interaction_token;
    logiList[queue].destination = dest;
    logiList[queue].item1 = item1;
    logiList[queue].item2 = item2;


}

void addToList(std::string ruser, std::string interaction_id, std::string interaction_token, std::string dest, std::string item1)
{
    logiList.push_back(logiRequest());
    //Get our queue number
    int queue = logiList.size() - 1;
    
    logiList[queue].ruser = ruser;
    logiList[queue].interaction_id = interaction_id;
    logiList[queue].interaction_token = interaction_token;
    logiList[queue].destination = dest;
    logiList[queue].item1 = item1;


}


std::string getToken() {
    std::string token;

    /*
                    First attempt to read the token from the BOT_TOKEN
       environment variable.
    */
    char const *env = std::getenv("BOT_TOKEN");
    if (env != nullptr) {
        token = std::string(env);
    } else {
        /*/
         * Read token from token file.
         * Tokens are required to communicate with Discord, and hardcoding
        tokens is a bad idea.
         * If your bot is open source, make sure it's ignore by git in your
        .gitignore file.
        /*/
        std::ifstream tokenFile("token.dat");
        if (!tokenFile) {
            return "";
        }
        safeGetline(tokenFile, token);
        tokenFile.close();
    }
    return token;
}

/*/
 * Source: https://stackoverflow.com/a/6089413/1526048
/*/
std::istream &safeGetline(std::istream &is, std::string &t) {
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf *sb = is.rdbuf();

    for (;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if (sb->sgetc() == '\n') {
                sb->sbumpc();
            }
            return is;
        case std::streambuf::traits_type::eof():
            // Also handle the case when the last line has no line ending
            if (t.empty()) {
                is.setstate(std::ios::eofbit);
            }
            return is;
        default:
            t += (char)c;
        }
    }
}

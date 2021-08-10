#include "include.hh"
#include "oauth.h"

#include <vector>

using json = nlohmann::json;
using std::vector;

namespace asio = boost::asio;
namespace dpp = discordpp;

std::string getToken();

std::istream &safeGetline(std::istream &is, std::string &t);

void addToList(std::string ruser, std::string token, std::string dest, std::string item1, std::string item2, 
               std::string item3, std::string item4, std::string item5);
void addToList(std::string ruser, std::string token, std::string dest, std::string item1, std::string item2, 
               std::string item3, std::string item4);
void addToList(std::string ruser, std::string token, std::string dest, std::string item1, std::string item2, 
               std::string item3);
void addToList(std::string ruser, std::string token, std::string dest, std::string item1, std::string item2);
void addToList(std::string ruser, std::string token, std::string dest, std::string item1);

void filter(std::string &target, const std::string &pattern);

struct logiRequest {
    std::string ruser;
    std::string token;

    std::string destination;

    std::string item1;
    std::string item2;
    std::string item3;
    std::string item4;
    std::string item5;

    std::string duser;
    bool accept = false;
    bool done = false;
    };

    //Create Logi List (GLOBAL)
    vector<logiRequest> logiList;

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

                //Submitting a Slash Command
                bot->callJson()
                    ->method("POST")
                    ->target("/applications/873355455806730281/guilds/873367900784832522/commands")
                    ->payload(requestCommand)
                    ->run();

             }});

    bot->prefix = "~";

    bot->respond("help", "Mention me and I'll echo your message back!");

    bot->respond("about", [&bot](json msg) {
        std::ostringstream content;
        content
            << "Sure thing, "
            << (msg["member"]["nick"].is_null()
                    ? msg["author"]["username"].get<std::string>()
                    : msg["member"]["nick"].get<std::string>())
            << "!\n"
            << "I'm a simple bot meant to demonstrate the Discord++ library.\n"
            << "You can learn more about Discord++ at "
               "https://discord.gg/VHAyrvspCx";
        bot->createMessage()
            ->channel_id(dpp::get_snowflake(msg["channel_id"]))
            ->content(content.str())
            ->run();
    });

    bot->respond("cancun", [&bot](json msg) {
        std::ifstream ifs("cancun.jpg", std::ios::binary);
        if (!ifs) {
            std::cerr << "Couldn't load file 'cancun.jpg'!\n";
            return;
        }
        ifs.seekg(0, std::ios::end);
        std::ifstream::pos_type fileSize = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        auto file = std::make_shared<std::string>(fileSize, '\0');
        ifs.read(file->data(), fileSize);

        bot->createMessage()
            ->channel_id(dpp::get_snowflake(msg["channel_id"]))
            ->content("We're goin' on a trip")
            ->filename("cancun.jpg")
            ->filetype("image/jpg")
            ->file(file)
            ->run();
    });

    bot->respond("channelinfo", [&bot](json msg) {
        bot->getChannel()
            ->channel_id(dpp::get_snowflake(msg["channel_id"]))
            ->onRead([&bot, msg](bool error, json res) {
                bot->createMessage()
                    ->channel_id(dpp::get_snowflake(msg["channel_id"]))
                    ->content("```json\n" + res["body"].dump(4) + "\n```")
                    ->run();
            })
            ->run();
    });

    // Create handler for the MESSAGE_CREATE payload, this receives all messages
    // sent that the bot can see.
    bot->handlers.insert(
        {"MESSAGE_CREATE", [&bot, &self](json msg) 
             { 
             // Scan through mentions in the message for self
             bool mentioned = false;
             for (const json &mention : msg["mentions"]) {
                 mentioned = mentioned or mention["id"] == self["id"];
             }
             if (mentioned) {
                 // Identify and remove mentions of self from the message
                 std::string content = msg["content"].get<std::string>();
                 unsigned int oldlength, length = content.length();
                 do {
                     oldlength = length;
                     content = std::regex_replace(
                         content,
                         std::regex(R"(<@!?)" + self["id"].get<std::string>() +
                                    R"(> ?)"),
                         "");
                     length = content.length();
                 } while (oldlength > length);

                 // Get the target user's display name
                 std::string name =
                     (msg["member"]["nick"].is_null()
                          ? msg["author"]["username"].get<std::string>()
                          : msg["member"]["nick"].get<std::string>());

                 std::cout << "Echoing " << name << '\n';

                //if(content == "/request" || "/Request")
                //{
                //    bot->createMessage()
                //    ->channel_id(dpp::get_snowflake(msg["channel_id"]))
                //    ->content("You would like to Request?")
                //    ->run();
                //}

                 // Set status to Playing "with [author]"
                 bot->send(3,
                           {{"game", {{"name", "with " + name}, {"type", 0}}},
                            {"status", "online"},
                            {"afk", false},
                            {"since", "null"}});
             }
         }});


    //Slash Handler 
    bot->handlers.insert({"INTERACTION_CREATE", [&bot](json msg){
        if (msg["data"]["name"].get<std::string>() == "request")
        {
            //std::cout << msg.dump(4);

            std::string interaction_id = msg["id"].get<std::string>();
            std::string interaction_token = msg["token"].get<std::string>();
            //std::string message = msg["data"]["options"][0]["value"].get<std::string>();
            std::string rUser = msg["member"]["user"]["username"].get<std::string>();
            std::string dest = msg["data"]["options"][0]["value"].get<std::string>();
            std::string item1 = msg["data"]["options"][1]["value"].get<std::string>();
            if(msg["data"]["options"][2]["value"] != nlohmann::detail::value_t::null)
            {
                std::string item2 = msg["data"]["options"][2]["value"].get<std::string>();
            }

            //{rUser, Interaction_token, destination, item1, item2, item3, item4, item5}
            addToList(rUser, interaction_token, dest, item1);
            
            json reply = {
                {"type", 4},
                {"data", {
                    {"content", "processing"}
                    }}
            };


            bot->callJson()
              ->method("POST")
              ->target("/interactions/" + interaction_id + "/" + interaction_token + "/callback")
              ->payload(reply)
              ->run();

            json embeds = {
                {"title", "Logi Request"},
                {"description", "etc"}
            };

             bot->createMessage()
                ->channel_id(dpp::get_snowflake(msg["channel_id"]))
                ->embed(embeds)
                ->run();

            
            
        }

        
        

        }});

    // Create Asio context, this handles async stuff.
    auto aioc = std::make_shared<asio::io_context>();

    // Set the bot up
    bot->initBot(6, token, aioc);

    // Run the bot!
    bot->run();

    return 0;
}

void addToList(std::string ruser, std::string token, std::string dest, std::string item1, std::string item2, 
               std::string item3, std::string item4, std::string item5)
{
    logiList.push_back(logiRequest());
    //Get our queue number
    int queue = logiList.size() - 1;
    
    logiList[queue].ruser = ruser;
    logiList[queue].token = token;
    logiList[queue].destination = dest;
    logiList[queue].item1 = item1;
    logiList[queue].item2 = item2;
    logiList[queue].item3 = item3;
    logiList[queue].item4 = item4;
    logiList[queue].item5 = item5;


}

void addToList(std::string ruser, std::string token, std::string dest, std::string item1, std::string item2, 
               std::string item3, std::string item4)
{
    logiList.push_back(logiRequest());
    //Get our queue number
    int queue = logiList.size() - 1;
    
      logiList[queue].ruser = ruser;
    logiList[queue].token = token;
    logiList[queue].destination = dest;
    logiList[queue].item1 = item1;
    logiList[queue].item2 = item2;
    logiList[queue].item3 = item3;
    logiList[queue].item4 = item4;


}

void addToList(std::string ruser, std::string token, std::string dest, std::string item1, std::string item2, 
               std::string item3)
{
    logiList.push_back(logiRequest());
    //Get our queue number
    int queue = logiList.size() - 1;
    
    logiList[queue].ruser = ruser;
    logiList[queue].token = token;
    logiList[queue].destination = dest;
    logiList[queue].item1 = item1;
    logiList[queue].item2 = item2;
    logiList[queue].item3 = item3;



}

void addToList(std::string ruser, std::string token, std::string dest, std::string item1, std::string item2)
{
    logiList.push_back(logiRequest());
    //Get our queue number
    int queue = logiList.size() - 1;
    
    logiList[queue].ruser = ruser;
    logiList[queue].token = token;
    logiList[queue].destination = dest;
    logiList[queue].item1 = item1;
    logiList[queue].item2 = item2;


}

void addToList(std::string ruser, std::string token, std::string dest, std::string item1)
{
    logiList.push_back(logiRequest());
    //Get our queue number
    int queue = logiList.size() - 1;
    
    logiList[queue].ruser = ruser;
    logiList[queue].token = token;
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

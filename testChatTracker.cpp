// ChatTracker tester
//
// The file commands.txt should contain a sequence of lines of the form
//   j userName chatName  which requests a call to join(userName, chatName)
//   t chatName           which requests a call to terminate(chatName)
//   c userName           which requests a call to contribute(userName)
//   l userName chatName  which requests a call to leave(userName, chatName)
//   l userName           which requests a call to leave(userName)

#include "ChatTracker.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
using namespace std;

const char* commandFileName = "commands.txt";

class SlowChatTracker
{
  public:
    void join(string user, string chat);
    int terminate(string chat);
    int contribute(string user);
    int leave(string user, string chat);
    int leave(string user);
  private:
    struct Info
    {
        Info(string u, string c) : user(u), chat(c), count(0) {}
        string user;
        string chat;
        int count;
    };
    vector<Info> m_info;
    vector<Info> m_usersWhoLeft;
};

struct Command
{
    static Command* create(string line, int lineno);
    Command(string line, int lineno) : m_line(line), m_lineno(lineno) {}
    virtual ~Command() {}
    virtual void execute(ChatTracker& ct) const = 0;
    virtual bool executeAndCheck(ChatTracker& ct, SlowChatTracker& sct) const = 0;
    string m_line;
    int m_lineno;
};

void extractCommands(istream& dataf, vector<Command*>& commands);
string testCorrectness(const vector<Command*>& commands);
void testPerformance(const vector<Command*>& commands);

int main()
{
    vector<Command*> commands;

      // Basic correctness test

    istringstream basicf(
    "j Fred Breadmaking\n"
    "j Ethel Breadmaking\n"
    "c Fred\n"
    "j Lucy Lint Collecting\n"
    "c Ethel\n"
    "c Fred\n"
    "c Fred\n"
    "j Ricky Lint Collecting\n"
    "j Fred Lint Collecting\n"
    "c Fred\n"
    "j Lucy Elbonian Politics\n"
    "c Fred\n"
    "c Ricky\n"
    "c Lucy\n"
    "j Lucy Breadmaking\n"
    "j Ethel Breadmaking\n"
    "c Lucy\n"
    "t Lint Collecting\n"
    "j Lucy Burmese Cats\n"
    "c Lucy\n"
    "j Lucy Worm Farming\n"
    "j Lucy Elbonian Politics\n"
    "l Lucy Breadmaking\n"
    "c Lucy\n"
    "l Ethel\n"
    "l Lucy\n"
    "j Ricky Lint Collecting\n"
    "c Ricky\n"
    "c Ricky\n"
    "l Ricky\n"
    "j Lucy Breadmaking\n"
    "l Fred\n"
    );
    extractCommands(basicf, commands);

    cout << "Basic correctness test: " << flush;
    cout << testCorrectness(commands) << endl;

    for (size_t k = 0; k < commands.size(); k++)
        delete commands[k];
    commands.clear();

      // Thorough correctness and performance tests

    ifstream thoroughf(commandFileName);
    if ( ! thoroughf)
    {
        cout << "Cannot open " << commandFileName
             << ", so cannot do thorough correctness or performance tests!"
             << endl;
        return 1;
    }
    extractCommands(thoroughf, commands);

    cout << "Thorough correctness test: " << flush;
    cout << testCorrectness(commands) << endl;

    cout << "Performance test on " << commands.size() << " commands: " << flush;
    testPerformance(commands);

    for (size_t k = 0; k < commands.size(); k++)
        delete commands[k];
}

struct JoinCmd : public Command
{
    JoinCmd(string u, string c, string line, int lineno)
     : Command(line, lineno), m_user(u), m_chat(c)
    {}
    virtual void execute(ChatTracker& ct) const
    {
        ct.join(m_user, m_chat);
    }
    virtual bool executeAndCheck(ChatTracker& ct, SlowChatTracker& sct) const
    {
        ct.join(m_user, m_chat);
        sct.join(m_user, m_chat);
        return true;
    }
    string m_user;
    string m_chat;
};

struct TerminateCmd : public Command
{
    TerminateCmd(string c, string line, int lineno)
     : Command(line, lineno), m_chat(c)
    {}
    virtual void execute(ChatTracker& ct) const
    {
        ct.terminate(m_chat);
    }
    virtual bool executeAndCheck(ChatTracker& ct, SlowChatTracker& sct) const
    {
        return ct.terminate(m_chat) == sct.terminate(m_chat);
    }
    string m_chat;
};

struct ContributeCmd : public Command
{
    ContributeCmd(string u, string line, int lineno)
     : Command(line, lineno), m_user(u)
    {}
    virtual void execute(ChatTracker& ct) const
    {
        ct.contribute(m_user);
    }
    virtual bool executeAndCheck(ChatTracker& ct, SlowChatTracker& sct) const
    {
        return ct.contribute(m_user) == sct.contribute(m_user);
    }
    string m_user;
};

struct Leave2Cmd : public Command
{
    Leave2Cmd(string u, string c, string line, int lineno)
     : Command(line, lineno), m_user(u), m_chat(c)
    {}
    virtual void execute(ChatTracker& ct) const
    {
        ct.leave(m_user, m_chat);
    }
    virtual bool executeAndCheck(ChatTracker& ct, SlowChatTracker& sct) const
    {
        return ct.leave(m_user, m_chat) == sct.leave(m_user, m_chat);
    }
    string m_user;
    string m_chat;
};

struct Leave1Cmd : public Command
{
    Leave1Cmd(string u, string line, int lineno)
     : Command(line, lineno), m_user(u)
    {}
    virtual void execute(ChatTracker& ct) const
    {
        ct.leave(m_user);
    }
    virtual bool executeAndCheck(ChatTracker& ct, SlowChatTracker& sct) const
    {
        return ct.leave(m_user) == sct.leave(m_user);
    }
    string m_user;
};

[[noreturn]]
void die(string msg, string field, int lineno)
{
    cout << msg << " " << field << " in line " << lineno << " of " << commandFileName << endl;
    exit(1);
}

Command* Command::create(string line, int lineno)
{
    istringstream iss(line);
    string field1;
    if (!(iss >> field1))
        return nullptr;
    if (field1.size() != 1)
        die("Bad command", field1, lineno);
    string field2;
    string field3;
    char ch;
    switch (field1[0])
    {
      case 'j':
        if (!(iss >> field2))
            die("Missing argument for ", field1, lineno);
        if (!(iss >> ch))
            die("Missing argument for ", field1, lineno);
        iss.unget();
        getline(iss, field3);
        return new JoinCmd(field2, field3, line, lineno);
      case 't':
        if (!(iss >> ch))
            die("Missing argument for ", field1, lineno);
        iss.unget();
        getline(iss, field2);
        return new TerminateCmd(field2, line, lineno);
      case 'c':
        if (!(iss >> field2))
            die("Missing argument for ", field1, lineno);
        return new ContributeCmd(field2, line, lineno);
      case 'l':
        if (!(iss >> field2))
            die("Missing argument for ", field1, lineno);
        if (!(iss >> ch))
            return new Leave1Cmd(field2, line, lineno);
        iss.unget();
        getline(iss, field3);
        return new Leave2Cmd(field2, field3, line, lineno);
    }
    die("Bad command", field1, lineno);
}

void extractCommands(istream& dataf, vector<Command*>& commands)
{
    string commandLine;
    int lineNumber = 0;
    while (getline(dataf, commandLine))
    {
        lineNumber++;
        Command* cmd = Command::create(commandLine, lineNumber);
        if (cmd != nullptr)
            commands.push_back(cmd);
    }
}

string testCorrectness(const vector<Command*>& commands)
{
    ChatTracker ct;
    SlowChatTracker sct;
    for (size_t k = 0; k < commands.size(); k++)
    {
          // Check if command agrees with our behavior

        if (!commands[k]->executeAndCheck(ct, sct))
        {
            ostringstream msg;
            msg << "*** FAILED *** line " << commands[k]->m_lineno
                << ": \"" << commands[k]->m_line << "\"";
            return msg.str();
        }
    }
    return "Passed";
}

//========================================================================
// Timer t;                 // create a timer and start it
// t.start();               // (re)start the timer
// double d = t.elapsed();  // milliseconds since timer was last started
//========================================================================

#include <chrono>

class Timer
{
  public:
    Timer()
    {
        start();
    }
    void start()
    {
        m_time = std::chrono::high_resolution_clock::now();
    }
    double elapsed() const
    {
        std::chrono::duration<double,std::milli> diff =
                          std::chrono::high_resolution_clock::now() - m_time;
        return diff.count();
    }
  private:
    std::chrono::high_resolution_clock::time_point m_time;
};

void testPerformance(const vector<Command*>& commands)
{
    double endConstruction;
    double endCommands;

    Timer timer;
    {
        ChatTracker ct;

        endConstruction = timer.elapsed();

        for (size_t k = 0; k < commands.size(); k++)
            commands[k]->execute(ct);

        endCommands = timer.elapsed();
    }

    double end = timer.elapsed();

    cout << end << " milliseconds." << endl
         << "   Construction: " << endConstruction << " msec." << endl
         << "       Commands: " << (endCommands - endConstruction) << " msec." << endl
         << "    Destruction: " << (end - endCommands) << " msec." << endl;
}

void SlowChatTracker::join(string user, string chat)
{
    vector<Info>::iterator p = m_info.end();
    while (p > m_info.begin())
    {
        p--;
        if (p->user == user  &&  p->chat == chat)
        {
            Info info = *p;
            m_info.erase(p);
            m_info.push_back(info);
            return;
        }
    }
    m_info.push_back(Info(user, chat));
}

int SlowChatTracker::terminate(string chat)
{
    int total = 0;
    vector<Info>::iterator p = m_info.begin();
    while (p != m_info.end())
    {
        if (p->chat == chat)
    {
            total += p->count;
            p = m_info.erase(p);
    }
    else
        p++;
    }
    p = m_usersWhoLeft.begin();
    while (p != m_usersWhoLeft.end())
    {
        if (p->chat == chat)
    {
            total += p->count;
            p = m_usersWhoLeft.erase(p);
    }
    else
        p++;
    }
    return total;
}

int SlowChatTracker::contribute(string user)
{
    vector<Info>::iterator p = m_info.end();
    while (p > m_info.begin())
    {
        p--;
        if (p->user == user)
        {
            p->count++;
            return p->count;
        }
    }
    return 0;
}

int SlowChatTracker::leave(string user, string chat)
{
    vector<Info>::iterator p = m_info.end();
    while (p > m_info.begin())
    {
        p--;
        if (p->user == user  &&  p->chat == chat)
        {
            int count = p->count;
            m_usersWhoLeft.push_back(*p);
            m_info.erase(p);
            return count;
        }
    }
    return -1;
}

int SlowChatTracker::leave(string user)
{
    vector<Info>::iterator p = m_info.end();
    while (p > m_info.begin())
    {
        p--;
        if (p->user == user)
        {
            int count = p->count;
            m_usersWhoLeft.push_back(*p);
            m_info.erase(p);
            return count;
        }
    }
    return -1;
}

//
//  ChatTracker.cpp
//  project 4
//
//  Created by Olivia on 5/29/20.
//  Copyright © 2020 Olivia. All rights reserved.
//

#include <functional>
#include <iostream>
#include "ChatTracker.h"

using namespace std;

class ChatTrackerImpl
{
public:
    ChatTrackerImpl(int maxBuckets);
    void join(string user, string chat);
    int terminate(string chat);
    int contribute(string user);
    int leave(string user, string chat);
    int leave(string user);
    ~ChatTrackerImpl();

private:
    struct Info
    {
        Info() : count(0){}
        Info(string u, string c, Info* n=nullptr, Info* p=nullptr) : user(u), chat(c), count(0), next(n), prev(p) {}
        string user;
        string chat;
        int count;
        Info* next;
        Info* prev;
    };

    struct HashTable
    {
        int max_buckets;
        Info **t;
        Info **top;

        HashTable(){}

        void generateHash(int buckets)
        {
            max_buckets=buckets;
            t = new Info * [max_buckets];
            top = new Info* [max_buckets];
            for (int i = 0; i< max_buckets; i++) {
                t[i] = nullptr;
                top[i]=nullptr;
            }
        }
    };

    HashTable m_info;
    int buckets;
    HashTable m_usersWhoLeft;
    HashTable m_chats;
    int chat_buckets;
    int deleteInfo(string user, string chat);
};


//this function deletes Info with user and chat from m_info and m_usersWhoLeft
//it is used in terminate
int ChatTrackerImpl::deleteInfo(string user, string chat)
{
    int total = 0;
    int hash_v = hash<string>()(user)%buckets;
    Info* p = m_info.t[hash_v];
    while(p!=nullptr)
    {
        Info* temp = p->next;

        if(p->chat == chat && p-> user == user)
        {
            total += p->count;
            //if this is the only Info node
            if(p->prev==nullptr && p->next == nullptr)
            {
                m_info.t[hash_v]=nullptr;
                m_info.top[hash_v]=nullptr;
            }
            else if(p->prev == nullptr)
            {
                m_info.t[hash_v] = p->next;
                p->next->prev = nullptr;
            }
            else if(p->next == nullptr)
            {
                p->prev->next = nullptr;
                m_info.top[hash_v] = p->prev;
            }
            else
            {
                p->prev->next = p->next;
                p->next->prev = p->prev;
            }
            delete p;
        }
        p = temp; //update p

    }

    p = m_usersWhoLeft.t[hash_v];
    while(p!=nullptr)
    {
        Info* temp = p->next;

        if(p->chat == chat && p-> user == user)
        {
            total += p->count;
            //if this is the only Info node
            if(p->prev==nullptr && p->next == nullptr)
            {
                m_usersWhoLeft.t[hash_v]=nullptr;
                m_usersWhoLeft.top[hash_v]=nullptr;
            }
            else if(p->prev == nullptr)
            {
                m_usersWhoLeft.t[hash_v] = p->next;
                p->next->prev = nullptr;
            }
            else if(p->next == nullptr)
            {
                p->prev->next = nullptr;
                m_usersWhoLeft.top[hash_v] = p->prev;
            }
            else
            {
                p->prev->next = p->next;
                p->next->prev = p->prev;
            }
            delete p;
        }
        p = temp; //update p

    }

    return total;
}

ChatTrackerImpl::ChatTrackerImpl(int maxBuckts)
{
    m_info.generateHash(maxBuckts);
    m_usersWhoLeft.generateHash(maxBuckts);
    buckets = maxBuckts;
    chat_buckets = maxBuckts;
    m_chats.generateHash(chat_buckets);
}



/* ================================================================= */
/* join(string user, string chat) implementation */

void ChatTrackerImpl::join(string user, string chat)
{
    //check if the user has joined this chat or not
    //if so, change the chat to the user's current chat
    //if not,
        //let the user join the chat, and the chat is the user's current chat

    unsigned long hash_v = hash<string>()(user)%buckets;
    Info *p = m_info.t[hash_v];

    //if there is no info in the bucket, the user has not joined the chat yet

    if(p==nullptr)
    {
        unsigned long chat_hash = hash<string>()(chat)%chat_buckets;
        Info* temp = m_chats.top[chat_hash];
        if(temp == nullptr)
        {
            m_chats.t[chat_hash] = new Info(user, chat);
            m_chats.top[chat_hash] = m_chats.t[chat_hash];
        }
        else
        {
            temp->next = new Info(user, chat, nullptr, temp);
            m_chats.top[chat_hash] = temp->next;
        }
        m_info.t[hash_v] = new Info(user, chat);
        m_info.top[hash_v] = m_info.t[hash_v];
        return;
    }

    while(p!=nullptr)
    {
        if(p->chat==chat && p->user==user)
            break;
        p=p->next;
    }


    //if the user has already joined the chat
    if(p!=nullptr)
    {
        //if the chat is the last chat (including the case that it is the only chat)
        if(p->next == nullptr)
        {
            //do nothing
        }

        //if the chat is the first chat but not the only chat
        else if (p->prev == nullptr)
        {
            m_info.t[hash_v] = p->next;
            p->next->prev = nullptr; // p->next is not nullptr, guaranteed by the previous if statement
            Info* temp = m_info.top[hash_v];
            temp->next = p;
            p->next = nullptr;
            p->prev = temp;
            m_info.top[hash_v] = p;
        }

        //else
        else
        {
            p->prev->next = p->next;
            p->next->prev = p->prev;
            Info* temp = p = m_info.top[hash_v];
            temp->next = p;
            p->prev = temp;
            p->next = nullptr;
            m_info.top[hash_v] = p;
        }
    }

    // p == nullptr: the user has not joined the chat
    else
    {
        //let the user join the chat, and the chat is the user's current chat

        if(m_info.top[hash_v]==nullptr)
        {
            m_info.t[hash_v]=new Info(user, chat);
            m_info.top[hash_v] = m_info.t[hash_v];
        }
        else
        {
            p = m_info.top[hash_v];
            p->next = new Info(user, chat, nullptr, p);
            m_info.top[hash_v] = p->next;
        }

        //updaate m_chats

        unsigned long chat_hash = hash<string>()(chat)%chat_buckets;
        Info* temp = m_chats.top[chat_hash];
        if(temp == nullptr)
        {
            m_chats.t[chat_hash] = new Info(user, chat);
            m_chats.top[chat_hash] = m_chats.t[chat_hash];
        }
        else
        {
            temp->next = new Info(user, chat, nullptr, temp);
            m_chats.top[chat_hash] = temp->next;
        }

    }

}


/* ================================================================= */
/* leave(string user) implementation */

int ChatTrackerImpl::leave(string user)
{
    unsigned long hash_v = hash<string>()(user)%buckets;
    Info* p = m_info.top[hash_v]; //now p points to the last Info in the linked list

    while(p != nullptr)
    {
        if(p->user == user)
            break;
        p = p->prev;
    }
    //at the end of this while loop, there can be two cases:
    //1. p==nullptr: the user is not associated with any chat
    //2. p!=nullptr: p points to the Info with the user, the user's current chat, and the user's contribution to that chat

    if(p==nullptr) //the user is not associated with any chat
    {
        return -1;
    }

    else // p!=nullptr
    {
        // remove first

        // if this is the only info in the bucket
        if(p->next == nullptr && p->prev == nullptr)
        {
            m_info.t[hash_v] = nullptr;
            m_info.top[hash_v] = nullptr;
        }

        // else if this is the first info in the bucket
        else if(p->prev == nullptr)
        {
            p->next->prev = nullptr;
            m_info.t[hash_v] = p->next;
        }

        // else if this is the last info in the bucket
        else if(p->next == nullptr)
        {
            p->prev->next = nullptr;
            m_info.top[hash_v] = p->prev;
        }

        // else
        else
        {
            p->next->prev = p->prev;
            p->prev->next = p->next;
        }

        // then insert
        if(m_usersWhoLeft.t[hash_v]==nullptr)
        {
            m_usersWhoLeft.top[hash_v] = p;
            m_usersWhoLeft.t[hash_v] = p;
            p->next = nullptr;
            p->prev = nullptr;

        }
        else
        {
            Info* temp = m_usersWhoLeft.top[hash_v];
            temp->next = p;
            p->next = nullptr;
            p->prev = temp;
            m_usersWhoLeft.top[hash_v]=p;
        }

        
        return p->count;
    }
}

/* ================================================================= */
/* leave(string user, string chat) implementation */

int ChatTrackerImpl::leave(string user, string chat)
{
    unsigned long hash_v = hash<string>()(user)%buckets;
    Info* p = m_info.top[hash_v];

    while(p != nullptr)
    {
        if(p->user == user && p->chat == chat)
            break;
        p = p->prev;
    }
    //at the end of this for loop, there can be two cases:
    //1. p==nullptr: the user is not associated with the chat indicated
    //2. p!=nullptr: p points to the Info with the user, the chat, and the user's contribution to that chat

    // if the user is not associated with the chat indicated
    if(p==nullptr)
        return -1;

    //else: the user is associated with the chat
    else
    {
        // remove first

        // if this is the only info in the bucket
        if(p->next == nullptr && p->prev == nullptr)
        {
            m_info.t[hash_v] = nullptr;
            m_info.top[hash_v] = nullptr;
        }

        // else if this is the first info in the bucket
        else if(p->prev == nullptr)
        {
            m_info.t[hash_v] = p->next;
            p->next->prev = nullptr;
        }

        // else if this is the last info in the bucket
        else if(p->next == nullptr)
        {
            p->prev->next = nullptr;
            m_info.top[hash_v] = p->prev;
        }

        // else
        else
        {
            p->next->prev = p->prev;
            p->prev->next = p->next;
        }

        // then insert

        if(m_usersWhoLeft.t[hash_v]==nullptr)
        {
            m_usersWhoLeft.top[hash_v] = p;
            m_usersWhoLeft.t[hash_v] = p;
            p->next = nullptr;
            p->prev = nullptr;

        }
        else
        {
            Info* temp = m_usersWhoLeft.top[hash_v];
            temp->next = p;
            p->next = nullptr;
            p->prev = temp;
            m_usersWhoLeft.top[hash_v]=p;
        }

        return p->count;
    }
}



/* ================================================================= */
/* contribute(string user) implementation */

int ChatTrackerImpl::contribute(string user)
{
    unsigned long hash_v = hash<string>()(user)%buckets;
    Info *p = m_info.top[hash_v];
    while(p!=nullptr)
    {
        if(p->user == user)
            break;
        p=p->prev;
    }
    // at the end of the while loop, there are two cases
    // 1. p==nullptr: the user is not associated with any chat
    // 2. p!=nullptr: p points to the Info with the user and the current chat

    //if the user is not associated with any chat
    if(p==nullptr)
        return 0;

    else
    {
        p->count++;
        return p->count;
    }
}


/* ================================================================= */
/* terminate(string chat) implementation */

int ChatTrackerImpl::terminate(string chat)
{
    int total = 0;

    unsigned long chat_hash = hash<string>()(chat)%chat_buckets;
    Info* p = m_chats.t[chat_hash];
    while(p != nullptr)
    {
        if(p->chat==chat)
            break;
        p = p->next;
    }
    // case 1: p == nullptr, the chat does not exist
    // case 2: p != nullptr, the chat exists (but probably has no user associated with it)

    // case 1: p == nullptr, the chat does not exist
    if(p == nullptr)
    {
        return 0;
    }

    // case 2: p != nullptr, the chat exists (but probably has no user associated with it)
    for(Info* i = m_chats.t[chat_hash]; i!= nullptr; i=i->next)
    {
        if(i->chat!=chat){continue;}

        total += deleteInfo(i->user, chat);
    }

    //delete relevant info in m_chats

    Info* j = m_chats.t[chat_hash];
    while(j!=nullptr)
    {
        if(j->chat != chat)
        {
            j=j->next;
            continue;
        }

        //delete j
        Info* temp = j->next;

        // if this is the only info in the bucket
        if(j->next == nullptr && j->prev == nullptr)
        {
            m_chats.t[chat_hash] = nullptr;
            m_chats.top[chat_hash] = nullptr;
        }

        // else if this is the first info in the bucket
        else if(j->prev == nullptr)
        {
            m_chats.t[chat_hash] = j->next;
            j->next->prev = nullptr;
        }

        // else if this is the last info in the bucket
        else if(j->next == nullptr)
        {
            j->prev->next = nullptr;
            m_chats.top[chat_hash] = j->prev;
        }

        // else
        else
        {
            j->next->prev = j->prev;
            j->prev->next = j->next;
        }

        delete j;
        j=temp;

    }


    return total;
}


/* ================================================================= */
/* ~ChatTrackerImpl() implementation */

ChatTrackerImpl::~ChatTrackerImpl()
{
    Info* p1;
    Info* p2;
    for(int i=0; i<buckets; i++)
    {
        p1=m_usersWhoLeft.t[i];
        p2=m_info.t[i];
//        delete m_info.top[i];
//        delete m_usersWhoLeft.top[i];
        Info* temp1 = p1;
        while(p1!=nullptr)
        {
            temp1=p1;
            p1=p1->next;
            delete temp1;
        }
        Info* temp2;
        while(p2!=nullptr)
        {
            temp2=p2;
            p2=p2->next;
            delete temp2;
        }

    }
    Info* p;
    for(int i = 0; i<chat_buckets; i++)
    {
        p=m_chats.t[i];
        Info* temp = p;
        while(p!=nullptr){
            temp = p;
            p = p->next;
            delete temp;
        }
    }
}






//*********** ChatTracker functions **************

// These functions simply delegate to ChatTrackerImpl's functions.
// You probably don't want to change any of this code.

ChatTracker::ChatTracker(int maxBuckets)
{
    m_impl = new ChatTrackerImpl(maxBuckets);
}

ChatTracker::~ChatTracker()
{
    delete m_impl;
}

void ChatTracker::join(string user, string chat)
{
    m_impl->join(user, chat);
}

int ChatTracker::terminate(string chat)
{
    return m_impl->terminate(chat);
}

int ChatTracker::contribute(string user)
{
    return m_impl->contribute(user);
}

int ChatTracker::leave(string user, string chat)
{
    return m_impl->leave(user, chat);
}

int ChatTracker::leave(string user)
{
    return m_impl->leave(user);
}


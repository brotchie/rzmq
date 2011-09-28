// -*- mode: C++; c-indent-level: 2; c-basic-offset: 2; tab-width: 8 -*-
///////////////////////////////////////////////////////////////////////////
// Copyright (C) 2011  Whit Armstrong                                    //
//                                                                       //
// This program is free software: you can redistribute it and/or modify  //
// it under the terms of the GNU General Public License as published by  //
// the Free Software Foundation, either version 3 of the License, or     //
// (at your option) any later version.                                   //
//                                                                       //
// This program is distributed in the hope that it will be useful,       //
// but WITHOUT ANY WARRANTY; without even the implied warranty of        //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         //
// GNU General Public License for more details.                          //
//                                                                       //
// You should have received a copy of the GNU General Public License     //
// along with this program.  If not, see <http://www.gnu.org/licenses/>. //
///////////////////////////////////////////////////////////////////////////

#ifndef SINK_HPP
#define SINK_HPP

#include <vector>
#include <iostream>
#include <pthread.h>
#include <zmq.hpp>
//#include <Rinternals.h>

class Sink {
private:
  typedef std::vector<char*> container;
  container results_;
  const char* address_;
  const size_t num_items_;  
  pthread_t worker_;
public:
  Sink(const char* address, size_t num_items):
    address_(address), num_items_(num_items)
  {
    pthread_create(&worker_, NULL, &Sink::start_thread, static_cast<void*>(this));
  }

  ~Sink() {
    for (container::iterator iter = results_.begin(); iter != results_.end(); iter++) {
      delete[] *iter;
    }
  }

  static void* start_thread(void *handle) {
    //All we do here is call the do_work() function
    reinterpret_cast<Sink*>(handle)->sink_routine();
    return static_cast<void*>(NULL);
  }

  void sink_routine() {
    size_t msgs_received(0);
    zmq::context_t context(1);
    zmq::socket_t receiver(context,ZMQ_PULL);
    try {
      receiver.connect(address_);
    } catch(std::exception& e) {
      std::cerr << e.what() << std::endl;
      // we don't want to execute the thread
      // if it can't connect
      return;
    }

    std::cout << "listening" << std::endl;
    do {
      zmq::message_t msg;
      receiver.recv(&msg);
      char* dest = new char[msg.size()];
      // if(dest == NULL) panic;
      results_.push_back(dest);
      memcpy(dest,msg.data(),msg.size());
      ++msgs_received;
      std::cout << "received:" << msgs_received << std::endl;
    } while(msgs_received < num_items_);

    zmq_close (receiver);
  }
};


#endif // SINK_HPP
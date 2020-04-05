#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <ctime>



int depqbf_run_with_timeout()
{
    auto start = std::chrono::system_clock::now();

    std::chrono::milliseconds span (1000);
    std::mutex m;
    std::condition_variable cv;
    int retValue;
    
    std::thread t([&m, &cv, &retValue]() 
    {
        retValue = system("./src/bash_script.sh");
        cv.notify_one();
    });
    
    t.detach();
    
    {
        std::unique_lock<std::mutex> l(m);
        if(cv.wait_for(l, span) == std::cv_status::timeout) { 
            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;
            std::cout << "Elapsed time is : " << elapsed_seconds.count() << "\n";
            throw std::runtime_error("Timeout");
        }
    }
   
    if(retValue == -1){
      std::cout << "SYTEM ERROR !!!\n";
     }

 //   std::cout << "Return Value is : " << retValue << "\n";
 //   auto end = std::chrono::system_clock::now();
    //std::chrono::duration<double> elapsed_seconds = end-start;
//    std::cout << end - start << "\n";
    return retValue;    
}

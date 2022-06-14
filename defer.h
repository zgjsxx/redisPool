#include <iostream>
#include <functional>
#include <mutex>

#define DEFER_LINENAME_CAT(name, line) name##line
#define DEFER_LINENAME(name, line) DEFER_LINENAME_CAT(name, line)
#define defer(deferFunction) RAIIDefer DEFER_LINENAME(DEFER_NAME_, __LINE__)(deferFunction)

class RAIIDefer
{
public:
	RAIIDefer(std::function<void()> fDeferFunction) {
		m_fDeferFunction = fDeferFunction;
	} 
	~RAIIDefer() {
		if (m_fDeferFunction)
		{
			m_fDeferFunction();
		}
	}
private:
	RAIIDefer() {};
	std::function<void()> m_fDeferFunction;
};
// int main()
// {   
//     std::cout << "lock" << std::endl;
// 	std::mutex mutex;
// 	mutex.lock();
// 	defer ( [&] {
//             std::cout << "unlock" << std::endl;
// 			mutex.unlock();
// 		}
// 	);  
//     /*
//         equals to 
//         RAIIDefer DEFER_NAME_36([&] { std::cout << "unlock" << std::endl; mutex.unlock(); })
//     */
//     std::cout << "doing something" << std::endl;
//     int res = -1;
//     {
//         if(res < 0){
//             std::cout << "will exit.." <<std::endl;    
//         }
//     }    
// }

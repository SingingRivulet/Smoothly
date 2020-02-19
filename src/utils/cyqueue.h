#ifndef CYQUEUE_H
#define CYQUEUE_H
#include <exception>

namespace smoothly{

template<typename T>
class circlequeue{
    private:
        int m_size;
        int m_front;
        int m_rear;
        T* m_data;
    public:
        circlequeue(int size=4096){
            m_front = m_rear = 0;
            m_data = new T[m_size = size];
        }
        ~circlequeue(){
            delete[] m_data;
        }
        bool isEmpty(){
            return m_front == m_rear;
        }
        bool isFull(){
            //m_front与m_rear均会移动，%size来判断，比如size = 10，m_rear = 9, m_front = 0的情况，需要考虑环形回环
            return m_front == (m_rear + 1) % m_size;
        }
        void push(T data){
            if (isFull()){
                throw std::range_error("当前环形队列已满，不允许继续push");
            }
            m_data[m_rear] = data;
            m_rear = (m_rear + 1) % m_size;
        }
        void pop(){
            if (isEmpty()){
                throw std::range_error("当前环形队列为空，不允许继续pop");
            }
            m_front = (m_front + 1) % m_size;
        }
        void popall(){
            if (isEmpty()){
                throw std::range_error("当前环形队列为空，不允许继续pop");
            }
            m_front = m_rear = 0;
        }
        T top(){
            if (isEmpty()){
                throw std::range_error("当前环形队列为空，没有top对象");
            }
            return m_data[m_front];
        }
};

}
#endif // CYQUEUE_H

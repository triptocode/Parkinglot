from pymongo import MongoClient
import datetime
import time

def insert_car_info(car_number, enter_time):

    car = client[db_name][col_name].find_one({'car_num':car_number})

    if car is None:
        car_info = dict({'car_num':car_number, 'enter_time':enter_time})
        client[db_name][col_name].insert_one(car_info)
        return True
    else:
        print('이미 주차장 안에 있는 차량입니다.')
        return False

def delete_car_info(car_number):

    car = client[db_name][col_name].find_one({'car_num':car_number})

    if car is None:
        print('없는 차량입니다.')
        return False
    else:
        enter_time = car['enter_time']
        car_number = car['car_num']
        client[db_name][col_name].delete_one({'car_num':car_number})

        # 요금 계산
        fee_and_exit_time = cal_fee(enter_time)
        print(f'주차 요금: {fee_and_exit_time[0]}원')

    return [car_number] + fee_and_exit_time

def cal_fee(enter_time):

    exit_time = datetime.datetime.today()
    exit_time = exit_time.strftime('%Y%m%d%H%M%S')

    enter_sec = int(enter_time) % 100
    exit_sec = int(exit_time) % 100

    enter_min = (int(enter_time) // 100) % 100
    exit_min = (int(exit_time) // 100) % 100

    enter_hour = (int(enter_time) // 10000) % 100
    exit_hour = (int(exit_time) // 10000) % 100

    enter_day = int(enter_time[:8]) % 100
    exit_day = int(exit_time[:8]) % 100

    enter_month = int(enter_time[:6]) % 100
    exit_month = int(exit_time[:6]) % 100

    enter_year = int(enter_time[:4])
    exit_year = int(exit_time[:4])

    # 초당 100원
    fee = 0
    sec = (exit_sec - enter_sec) * 1
    min = (exit_min - enter_min) * 60 * 1
    hour = (exit_hour - enter_hour) * 60 * 60 * 1
    day = (exit_day - enter_day) * 24 * 60 * 60 * 1

    fee += sec + min + hour + day

    print(f'입장 시간: {enter_year}년 {enter_month}월 {enter_day}일 {enter_hour}시 {enter_min}분 {enter_sec}초')
    print(f'출차 시간: {exit_year}년 {exit_month}월 {exit_day}일 {exit_hour}시 {exit_min}분 {exit_sec}초')

    # 요금과 출장 시간 반환
    return [fee, exit_time]

def insert_park_info():
    occupied_space = client[db_name][col_name].count_documents({})
    total_space = 5  # Assuming total spots are 5
    available_space = total_space - occupied_space

    park_info = {
        'current_time': current_time,  
        'total_space': total_space,
        'occupied_space': occupied_space,
        'available_space': available_space,
    }

    if occupied_space >=total_space:
        print('=== 만차입니다. Parking lot is already full ===')
        return False
    else:
        client[db_name][col_name2].insert_one(park_info)
        print(f'전체 주차공간: {total_space}, 사용한 주차공간: {occupied_space}, 남은 주차공간: {available_space}')
        return True


host = '127.0.0.1' 
# host = '10.10.59.223' # host ip : db server
port = 27017
# username = 'rpi'
# password = '1234'   

# client = MongoClient(host=host, port=port, username=username, password=password)
client = MongoClient(host=host, port=port)

db_name = 'parkinglot'
col_name = 'license'
col_name2 ='park_space'

# 현재 시간을 YYYYmmddHHMMSS 형식으로 얻는 방법환
current_time = datetime.datetime.today()
current_time = current_time.strftime('%Y%m%d%H%M%S')

if __name__ == '__main__':

     # 차량 정보 데이터 삽입 
    # insert_car_info('111A1111',current_time)
    # insert_car_info('222A2222',current_time)
    insert_car_info('333A3333',current_time)
    insert_car_info('444A4444',current_time)
    insert_car_info('555A5555',current_time)

    # 차량 정보 데이터 조회 
    datas = client[db_name][col_name].find()
    for data in datas:
        print(data)
    
   # time.sleep(1)

    # 차량 정보 데이터 삭제
    # info = delete_car_info('111A1111')
    # print(info)

    # 주차 공간 정보 데이터 삽입 
    insert_park_info()    
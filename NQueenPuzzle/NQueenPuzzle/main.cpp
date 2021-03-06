#include "stdafx.h"
#include "HillClimbing.h"
#include "AStar.h"

//A*로 해결
//언덕오르기로 해결
//무작위 재시작 언덕오르기로 해결 (옵션으로 횡이동 ON/OFF)

using namespace std;



int main(void)
{
	int num = 3;
	int option = -1;

	cout << "NQueen 퍼즐 만들기" << endl;
	while (num < 4) {
		cout << "판 크기 입력 (4이상의 정수) : ";
		cin >> num;

		if (num < 4)
			cout << "다시 입력하세요" << endl;
		else
			cout <<endl<< num << "Queen 퍼즐을 생성합니다" << endl << endl;
	}
	

	cout << "어떤 방식으로 찾을까요?" << endl;
	cout <<endl<< "Astar - 0" << endl << "HillClimbing - 1" << endl
		<< "HillClimbing and autoRestart - 2" << endl << "HillClimbing ant autoRestart and SideWay - 3" << endl << endl;
	
	while (option < 0 || option >3) {
		cout << "입력(0~3사이 정수) : ";
		cin >> option;

		if (option < 0 || option >3)
			cout << "다시 입력하시요" << endl;
	}
	cout << endl;

	DWORD preTime = timeGetTime();
	DWORD currTIme = -1;
	if (option == OPTION::ASTAR) {
		AStar aStar(num);
		aStar.Search();
		currTIme = timeGetTime();
	}
		
	else {
		//판생성
		Board board(num);

		//판에 퀸 배치
		for (int i = 0; i < num; ++i)
			board.resetQueen(i, i, i);

		HillClimbing hill(&board, num, option);
		hill.Search();
		currTIme = timeGetTime();
	}

	cout << "소요시간 : " << (currTIme - preTime)*0.001 << "sec"<<std::endl;

	return 0;
}
#include <iostream>

void header() {
  std::cout << "  ____ ____  ____  _____ _____ ____ __   __     \n";
  std::cout << " / __/  ___|/ __ `|  _  ` ____/ ___`  ` / /     \n";
  std::cout << "| |   `___ ` |  | | |_| |  __|`___ ` `   /      \n";
  std::cout << "| |__ ___) | |__| | ___/| |___ ___) | | |       \n";
  std::cout << " `___` ____/`____/|_|   |_____|___ /  |_|       \n";
  std::cout << " -----------------------------------------------\n";

  std::cout << "\033[32m";
  std::cout << "Hello, Welcome to CSOPESY commandline!\n";
  std::cout << "\033[0m";

  std::cout << "\033[93m";
  std::cout << "Type 'exit' to quit, 'clear' to clear the screen\n";
  std::cout << "\033[0m"; 
}

void initialize() {
  std::cout << "'initialize' command recognized. Doing something.\n";
}

void screen() {
  std::cout << "'screen' command recognized. Doing something.\n";
}

void scheduler_test() {
  std::cout << "'scheduler-test' command recognized. Doing something.\n";
}

void scheduler_stop() {
  std::cout << "'scheduler_stop' command recognized. Doing something.\n";
}

void report_util() {
  std::cout << "'report_util' command recognized. Doing something.\n";
}

void clear() {
  std::cout << "'clear' command recognized. Clearing screen.\n";
  std::cout << "\033[2J\033[1;1H"; // alternative to system("cls")
  header();
}

void exit() {
  std::cout << "'exit' command recognized. Exiting\n";
  exit(0);
}

int main() {

  header();
  std::string cmd;

  while (true) {
    std::cout << "Enter a command: ";
    std::cin >> cmd;

    //  initialize , screen, scheduler-test, scheduler-stop , report-util
    if (cmd == "initialize") {
      initialize();
    } else if (cmd == "screen") {
      screen();
    } else if (cmd == "scheduler-test") {
      scheduler_test();
    } else if (cmd == "scheduler-stop") {
      scheduler_stop();
    } else if (cmd == "report-util") {
      report_util();
    } else if (cmd == "clear") {
      clear();
    } else if (cmd == "exit") {
      exit();
    }
  }
}
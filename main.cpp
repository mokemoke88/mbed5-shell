/**
 * @file main.cpp
 * @brief
 * @author kshibata@seekers.jp
 * @date 2016-11-08
 * @par history
 * - 2016-11-08 16:54:23
 *  - first.
 */

#include <vector>
#include <string>
#include "mbed.h"
#include "seekers/mbed/rs485serial.hpp"

#include "vars.h"

/**
 * @brief スタックテンプレート
 */
template <typename T>
class stack_t
{
  std::vector<T> stack_;
  const T base_entry_;
public:
  explicit stack_t(const T& base_entry) :
    base_entry_(base_entry)
  {}

  T pop(void)
  {
    if(stack_.empty()) return base_entry_;
    T t = stack_.front();
    stack_.erase(stack_.begin());
    return t;
  }

  T peek(void)
  {
    if(stack_.empty()) return base_entry_;
    return stack_.front();
  }

  void push(const T& src)
  {
    stack_.push_back(src);
  }
};

// シーンエントリ型
typedef void (*scene_entry_t)(void);

// コマンド文字列 実行関数構造体
typedef struct {
  const char* key;
  scene_entry_t entry;
} menu_t;


// シーンエントリ関数
void top_level_menu_entry(void);
void run_entry(void);
void baud_entry(void);
void format_entry(void);
void bin_dump_entry(void);
void hex_dump_entry(void);

// シーンループ関数
void top_level_menu_loop(void);
void run_loop(void);
void baud_setup_loop(void);
void format_setup_loop(void);
void bin_dump_loop(void);
void hex_dump_loop(void);

// シーンスタック
stack_t<scene_entry_t> scene_stack_(run_entry);

// コマンド入力時バッファ
std::string cmd_buf_;

// コマンド - エントリ関数配列
menu_t top_menu_[] = {
  { "1", &baud_entry },
  { "2", &format_entry },
  { "Db", &bin_dump_entry },
  { "Dh", &hex_dump_entry },
  { NULL, NULL }
};

// マーク出力用Ticker
Ticker mark_printer;

// マーク出力関数
void mark_print(void)
{
  pc.printf("\r\n === MARK === \r\n");
}

/**
 * @brief コマンド分離
 */
std::string parse_cmd(const std::string& src)
{
  std::string::size_type npos = src.find_first_of(" ");
  if( npos == std::string::npos )
    return src;
  else
    return src.substr(0, npos);
}

/**
 * @brief コマンド文字列から実行関数の選択
 * @param cmd : 対象のコマンド文字列
 * @param menu : メニュー配列
 * @param current : 現在のシーンエントリ関数
 */
void menu_select(const std::string& cmd, const menu_t* menu, const scene_entry_t& current)
{
  for(int ii = 0; menu[ii].key != NULL; ++ii){
    if(cmd.compare(menu[ii].key) == 0 ){
      scene_stack_.push(current);
      menu[ii].entry();
      return;
    }
  }
  pc.printf("[ERROR] invalid selection.\r\n");
  current();
}

/**
 * @brief トップレベルメニューの表示
 */
void show_top_level(void)
{
  pc.printf("1) Uart Baudrate Setting [%6dbps]\r\n", uart_baud_);
  pc.printf("2) Uart Format Setting   [%d%s%d]\r\n",
            uart_bits_,
            (uart_parity_ == Serial::None ) ? "N" :
            (uart_parity_ == Serial::Even ) ? "E" : "O",
            uart_stop_bits_);
  pc.printf("R) Run Main Program.\r\n");
  pc.printf("Db) Run Uart Dump[BIN].\r\n");
  pc.printf("Dh) Run Uart Dump[HEX]. \r\n");
}

/**
 * @brief トップレベルメニューのエントリ
 */
void top_level_menu_entry(void)
{
  show_top_level();
  runtime_loop = top_level_menu_loop;
}

/**
 * @brief トップレベルのメニュー処理
 */
void top_level_menu_loop(void)
{
  int ch = pc.getc();
  if(ch >= 0)
  {
    if(ch == '\r'){
      pc.putc('\r'); pc.putc('\n');
      std::string cmd = parse_cmd(cmd_buf_);
      menu_select(cmd, top_menu_, top_level_menu_entry);
      cmd_buf_.clear();
    }else{
      pc.putc(ch);
      cmd_buf_.push_back(ch);
    }
  }
}

/**
 * @brief プログラム動作直前設定
 */
void run_entry()
{
  uart.baud(uart_baud_);
  uart.format(uart_bits_, uart_parity_, uart_stop_bits_);

  pc.printf("Run Main process.\r\n");
  pc.printf("Uart: %6dbps %d%s%d.\r\n",
            uart_baud_,
            uart_bits_,
            (uart_parity_ == Serial::None ) ? "N" :
            (uart_parity_ == Serial::Even ) ? "E" : "O",
            uart_stop_bits_
  );
  runtime_loop = run_loop;
}

/**
 * 実際の動作
 */
void run_loop(void)
{
  Thread::wait(1000);
  pc.printf("Hello.\r\n");
  uart.printf("Hello.\r\n");
}

/**
 * @brief ボーレート変更エントリ関数
 */
void baud_entry(void)
{
  pc.printf("Uart Baudrate setup. now [%6dbps]\r\n>", uart_baud_);
  runtime_loop = &baud_setup_loop;
}

/**
 * @brief ボーレート設定
 */
void baud_setting(const std::string& cmd)
{
  int baud = atoi(cmd.c_str());
  switch(baud){
  case 4800:
  case 9600:
  case 19200:
  case 38400:
  case 115200:
    uart_baud_ = baud;
    pc.printf("change baudrate.\r\n");
    break;
  default:
    pc.printf("[ERROR] invalidate baudrate.\r\n");
  }
  (scene_stack_.pop())();
}

/**
 * @brief ボーレートコマンドの受付
 */
void baud_setup_loop(void)
{
  int ch = pc.getc();
  if(ch >= 0)
  {
    if(ch == '\r'){
      pc.putc('\r'); pc.putc('\n');
      std::string cmd = parse_cmd(cmd_buf_);
      baud_setting(cmd);
      cmd_buf_.clear();
    }else{
      pc.putc(ch);
      cmd_buf_.push_back(ch);
    }
  }
}

/**
 * @brief フォーマット変更 エントリ関数
 */
void format_entry(void)
{
  pc.printf("Uart format setup. now [%d%s%d]\r\n>",
            uart_bits_,
            (uart_parity_ == Serial::None ) ? "N" :
            (uart_parity_ == Serial::Even ) ? "E" : "O",
            uart_stop_bits_ );
  runtime_loop = &format_setup_loop;
}

/**
 * @brief フォーマット設定
 */
void format_setting(const std::string& cmd)
{
  if( cmd.size() != 3 )
    pc.printf("[ERROR] invalid format.\r\n");
  const bool test_bits = (cmd[0] == '7' || cmd[0] == '8');
  const bool test_parity = (cmd[1] == 'N' || cmd[1] == 'n'
                            || cmd[1] == 'O' || cmd[1] == 'o'
                            || cmd[1] == 'E' || cmd[1] == 'e'
  );
  const bool test_stop_bits = (cmd[2] == '1' || cmd[2] == '2');
  if( test_bits && test_parity && test_stop_bits ){
    uart_bits_ = (cmd[0] == '7') ? 7 : 8;
    uart_parity_ = (cmd[1] == 'N' || cmd[1] == 'n' ) ? Serial::None :
      (cmd[1] == 'O' || cmd[1] == 'o' ) ? Serial::Odd : Serial::Even;
    uart_stop_bits_ = (cmd[2] == '1') ? 1 : 2;
    pc.printf("change format.\r\n");
  }else{
    pc.printf("[ERROR] invalid format.\r\n");
  }
  (scene_stack_.pop())();
}

/**
 * @brief フォーマットコマンドの受付
 */
void format_setup_loop(void)
{
  int ch = pc.getc();
  if(ch >= 0)
  {
    if(ch == '\r'){
      pc.putc('\r'); pc.putc('\n');
      std::string cmd = parse_cmd(cmd_buf_);
      format_setting(cmd);
      cmd_buf_.clear();
    }else{
      pc.putc(ch);
      cmd_buf_.push_back(ch);
    }
  }
}

/**
 * @brief  バイナリダンプ エントリ関数
 */
void bin_dump_entry(void)
{
  uart.baud(uart_baud_);
  uart.format(uart_bits_, uart_parity_, uart_stop_bits_);

  pc.printf("=== Uart BIN Dump Mode ===\r\n");
  pc.printf("Uart: %6dbps %d%s%d.\r\n",
            uart_baud_,
            uart_bits_,
            (uart_parity_ == Serial::None ) ? "N" :
            (uart_parity_ == Serial::Even ) ? "E" : "O",
            uart_stop_bits_
  );
  mark_printer.attach(callback(mark_print), 10);
  runtime_loop = &bin_dump_loop;
}

void bin_dump_loop(void)
{
  if(uart.readable()){
    int ch = uart.getc();
    if(ch >= 0)
      pc.putc(ch);
  }
}

/**
 * @brief HEXダンプ エントリ関数
 */
void hex_dump_entry(void)
{
  uart.baud(uart_baud_);
  uart.format(uart_bits_, uart_parity_, uart_stop_bits_);

  pc.printf("=== Uart Hex Dump Mode ===\r\n");
  pc.printf("Uart: %6dbps %d%s%d.\r\n",
            uart_baud_,
            uart_bits_,
            (uart_parity_ == Serial::None ) ? "N" :
            (uart_parity_ == Serial::Even ) ? "E" : "O",
            uart_stop_bits_
  );
  mark_printer.attach(callback(mark_print), 10);
  runtime_loop = &hex_dump_loop;
}

void hex_dump_loop(void)
{
  if(uart.readable()){
    int ch = uart.getc();
    if(ch >= 0){
      pc.printf("%02xh ", ch);
    }
  }
}

/**
 * @brief 初期設定
 */
void setup(void)
{
  pc.baud(115200);
  pc.format(8, Serial::None, 1);
  pc.printf("\r\n=== mbed5-shell ===\r\n");
  pc.printf("Version: " SELF_VERSION " BUILD at " __DATE__ " " __TIME__ "\r\n");

  uart.baud(uart_baud_);
  uart.format(uart_bits_, uart_parity_, uart_stop_bits_);

  pc.printf("USB Serial: 115200bps 8N1.\r\n");
  pc.printf("Uart      : %6dbps %d%s%d.\r\n",
            uart_baud_,
            uart_bits_,
            (uart_parity_ == Serial::None ) ? "N" :
            (uart_parity_ == Serial::Even ) ? "E" : "O",
            uart_stop_bits_
  );

  top_level_menu_entry();
  runtime_loop = top_level_menu_loop;
}

/**
 * @brief エントリポイント
 */
int main()
{
  setup();
  for(;;)
    runtime_loop();
}

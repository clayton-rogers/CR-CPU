#include <vga.h>
#include <unistd.h>
#include <cr-os.h>
#include <print_num.h>

// ===================================================== //
// ==================== PONG =========================== //
// ===================================================== //
// This program is a simple pong game.
// To play:
//   Player A:
//     w = up
//     s = down
//   Player B:
//     o = up
//     l = down
//
// Game controls:
//   q = quit
//   r = restart
//   p = pause
//
// Recommended usage:
// run: make pong.srec && cat pong.srec
// then paste into PuTTY terminal
// ===================================================== //


// CONSTANTS
int player_char = '|';
int ball = 'O';

int height = 24;
int width = 80;
int paddle_height = 5;


// === GAME STATE ===
// 1 character in x == 4 sub-positions
// 1 character in y == 8 sub-positions
// x width = 4 * 80 = 320
// y height = 8 * 24 = 192
int ball_p_x = 10;
int ball_p_y = 10;
int ball_v_x = 1;
int ball_v_y = 1;
int player_a_pos = 0;
int player_b_pos = 0;

// === GAME SPEEDUP ===
int start_sleep_ms = 35;
int sleep_ms = 35;
int start_frame_count = 60;
int frame_count = 60;

// === META STATE ===
int should_quit = 0;
int score_a = 0;
int score_b = 0;


int get_rand(); // from asm

int restart_ball() {
	int dir = get_rand();
	if (dir == 0) {
		ball_v_x = 1;
		ball_v_y = 1;
	}
	if (dir == 1) {
		ball_v_x = -1;
		ball_v_y = -1;
	}
	if (dir == 2) {
		ball_v_x = -1;
		ball_v_y = 1;
	}
	if (dir == 3) {
		ball_v_x = 1;
		ball_v_y = -1;
	}
}

int restart() {
	sleep_ms = start_sleep_ms;
	frame_count = start_frame_count;
	ball_p_x = 160;
	ball_p_y = 96;
	restart_ball();
}

int collide_A() {
	if (ball_p_x != 0) {
		return 0;
	}
	int pos_y = ball_p_y >> 3;
	return (pos_y >= player_a_pos)
		&& (pos_y <= (player_a_pos + paddle_height));
}

int collide_B() {
	if (ball_p_x != 319) {
		return 0;
	}
	int pos_y = ball_p_y >> 3;
	return (pos_y >= player_b_pos)
		&& (pos_y <= (player_b_pos + paddle_height));
}

int update_pos() {
	// propagate ball
	ball_p_x = ball_p_x + ball_v_x;
	ball_p_y = ball_p_y + ball_v_y;

	// detect collisions with paddle and walls
	if (ball_p_x == -1) {
		score_b = score_b + 1;
		restart();
	}
	if (ball_p_x == 320) {
		score_a = score_a + 1;
		restart();
	}
	if (ball_p_y == 0 ||
		ball_p_y == 192) {
		ball_v_y = -ball_v_y;
	}
	if (collide_A() || collide_B()) {
		ball_v_x = -ball_v_x;
	}

	while (0 != __read_uart_rx_size()) {
		int input = __read_uart();
		if (input == 'q') {
			should_quit = 1;
		}
		if (input == 'p') {
			// pause, wait for p again
			set_vga_cursor(820);
			write_vga_char('p');
			while (__read_uart() != 'p') { }
		}
		if (input == 'r') {
			score_a = 0;
			score_b = 0;
			restart();
		}
		if (input == 'w' && player_a_pos != 0) { // a up
			player_a_pos = player_a_pos - 1;
		}
		if (input == 's' && player_a_pos != (height - paddle_height)) {
			player_a_pos = player_a_pos + 1;
		}
		if (input == 'o' && player_b_pos != 0) {
			player_b_pos = player_b_pos - 1;
		}
		if (input == 'l' && player_b_pos != (height - paddle_height)) {
			player_b_pos = player_b_pos + 1;
		}
	}
}

int print_paddle(int char_pos) {

	for (int i = 0; i < paddle_height; i = i + 1) {
		set_vga_cursor(char_pos);
		write_vga_char(player_char);
		char_pos = char_pos + width;
	}
}

int print_ball() {
	int pos_x = ball_p_x >> 2;
	int pos_y = ball_p_y >> 3;
	int pos = pos_y * 80 + pos_x;
	set_vga_cursor(pos);
	write_vga_char(ball);
}

int print_scores() {
	set_vga_cursor(35); // 0, 40
	write_vga_char('A');
	write_vga_char(':');
	__print_num(score_a);
	write_vga_char(' ');
	write_vga_char('B');
	write_vga_char(':');
	__print_num(score_b);
}

int print_screen() {
	clear_screen();

	print_paddle(player_a_pos * 80);
	print_paddle(player_b_pos * 80 + 79);
	print_ball();
	print_scores();
}

int main() {
	clear_screen();
	restart();

	while (!should_quit) {
		update_pos();
		print_screen();
		msleep(sleep_ms);
		frame_count = frame_count - 1;
		if (frame_count == 0 && sleep_ms != 1) {
			sleep_ms = sleep_ms - 1;
			frame_count = start_frame_count;
		}
	}

	set_vga_cursor(83);
	write_vga_char('d');
	write_vga_char('o');
	write_vga_char('n');
	write_vga_char('e');
}

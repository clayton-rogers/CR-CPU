#include "simulator.h"

// To be able to build the below program loader.
#include "assembler.h"
#include "linker.h"
#include "utilities.h"

#include <stdexcept>

std::string program_loader_s =
".constant 0x1000 RAM_SIZE\n"
".constant 0x0200 PROGRAM_ADDR\n"

"# Preamble to setup stack pointer\n"
"loadi sp .RAM_SIZE\n"
"loadi.h sp .RAM_SIZE\n"
"sub sp, 1\n"
"loadi ra .end\n"
"loadi.h ra .end # load the return address for the program\n"
"push ra\n"

"loadi ra .PROGRAM_ADDR\n"
"loadi.h ra .PROGRAM_ADDR\n"
"push ra\n"
"ret # jump to last pushed, i.e PROGRAM_ADDR\n"

".end: # when the program returns halt\n"
"halt\n"
;


Simulator::Simulator():
	bus(std::make_shared<Simulator_Bus>()),
	core(bus),
	ram(bus),
	io(bus, 0x8100),
	timer(bus, 0x8200),
	uart(bus, 0x8300),
	vga(bus, 0x8400),
	spi(bus, 0x8500)
{}

void Simulator::load_sim_overlay()
{
	// By default setup stack and jump to 0x0200, then halt when return is called
	auto program_loader_o = assemble(program_loader_s);
	auto program_loader_exe = link({ program_loader_o }, 0); // Load PL to 0x0000
	load(program_loader_exe);
}

void Simulator::load(const Object::Object_Container& obj)
{
	if (obj.contents.index() != Object::Object_Container::EXECUTABLE) {
		throw std::logic_error("Tried to load invalid object into simulator");
	}
	const auto exe = std::get<Object::Executable>(obj.contents);
	ram.load_ram(exe.load_address, exe.machine_code);
}

void Simulator::step(bool output_state)
{
	core.step();
	ram.step();
	io.step();
	timer.step();
	uart.step();
	vga.step();
	spi.step();

	if (output_state) {
		dump_state();
	}

	bus->check_bus_state_and_reset();
}

void Simulator::dump_state()
{
	Simulator_Core::Reg r = core.get_reg();
	f
		<< u16_to_string(r.ra) << " "
		<< u16_to_string(r.rb) << " "
		<< u16_to_string(r.rp) << " "
		<< u16_to_string(r.sp) << " "
		<< u16_to_string(core.get_pc()) << " "
		<< std::endl;
}

static void output_header(std::ofstream* f)
{
	auto& file = *f;
	file << "ra   rb   rp   sp   pc_n " << std::endl;
}

void Simulator::run_until_halted(const int number_steps, bool output_state)
{
	steps_remaining = number_steps;
	if (output_state) {
		f = std::ofstream("sim_out.txt");
		output_header(&f);
	}
	while (!get_state().is_halted && steps_remaining != 0) {
		--steps_remaining;
		step(output_state);
	}
}

Simulator::State Simulator::get_state()
{
	State s{
		core.get_reg().ra,
		core.get_pc(),
		core.is_halted,

		io.output,
		steps_remaining,
		uart.get_output()
	};

	return s;
}

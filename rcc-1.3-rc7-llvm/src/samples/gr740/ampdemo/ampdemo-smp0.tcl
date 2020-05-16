after 200
bp del
reset
# Forget any old symbols
foreach c {cpu0 cpu1 cpu2 cpu3} {
	silent load clear $c
	silent symbols clear $c
}

# CPU0-2: RTEMS SMP
# First 16 MiB
# APBUART1, GPTIMER1
load smp-tick-cpu0 cpu0
foreach c {cpu0 cpu1 cpu2} {
	silent stack 0x00fffff0 $c
	silent ep 0x00000000 $c
}

# CPU3: BCC
# Second 16 MiB
# APBUART0, GPTIMER0
load sp-tick-cpu3 cpu3
silent stack 0x01fffff0 cpu3
silent ep 0x01000000 cpu3

silent forward disable uart0
silent forward disable uart1
silent forward enable uart0
silent forward enable uart1

set paranoid true
if {$paranoid} {
	# Break on access to unassigned resource
	# APBUART0.status
	bp watch 0xff900004 cpu0
	bp watch 0xff900004 cpu1
	bp watch 0xff900004 cpu2

	# APBUART1.status
	bp watch 0xff901004 cpu3

	# GPTIMER0.timer0.ctrl
	bp watch 0xff908018 cpu0
	bp watch 0xff908018 cpu1
	bp watch 0xff908018 cpu2

	# GPTIMER1.timer0.value
	bp watch 0xff909010 cpu0
	bp watch 0xff909010 cpu1
	bp watch 0xff909010 cpu2

	bp watch 0xff909018 cpu0
	bp watch 0xff909018 cpu1
	bp watch 0xff909018 cpu2
}

# Define hook procedures
proc myhook1 {} {
	puts "Configuring interrupt controller"
	set ::irqmp0::0::actrl 0
	# Make the instance use separate IRQ controllers
	# CPU[0,1,2] uses controller 0, CPU[3] uses controller 3
	set ::irqmp0::0::select1 0x00030000
	set ::irqmp0::0::select2 0
	# Lock out other CPUs
	set ::irqmp0::0::actrl 1

	# Clear mask for all interrupt controllers. It prevents CPU power up
	# by pending interrupt at GRMON run.
	set ::irqmp0::0::cpumask0 0
	set ::irqmp0::0::cpumask1 0
	set ::irqmp0::0::cpumask2 0
	set ::irqmp0::3::cpumask3 0
	irq routing
	# Remove us so GRMON 'cont' will work
	unset ::hooks::preexec
}

cpu active 0

puts ""
forward
ep
stack

set ::hooks::preexec ::myhook1
puts "use 'run' to start example"


Project 3: Thread Scheduler
======================

INSTRUCTIONS
============

1. OVERVIEW
===========

In this project, you will write a user-mode thread scheduler.  The basic purpose
of a scheduler is to multiplex use of the computer across several threads
of execution.  This project deals with two different scheduling policies: FIFO
and Round Robin.  You will implement both, for use in a simple cooperative
multi-threading system.  Along the way, you'll also learn about implementing
object-oriented constructs in low-level procedural languages like C.

This assignment consists of implementing the core functionality of the
scheduler (Step 4) and answering 10 questions (Step 5).  Code for
Step 4 goes in sched_impl.c and sched_impl.h.


2. THEORY OF OPERATION
======================

The given code in the project defines the skeleton of a scheduler together with a
parameterized dummy workload.  The idea is when you run the MP, you specify
a scheduling policy, scheduler queue size, some number of worker threads to
create, and, optionally, the number of iterations for which the worker
threads should run.  The basic code that parses command line arguments and
creates these worker threads is provided in the MP, but you must implement
the core synchronization and scheduling operations.

As provided, the project only includes the "dummy" scheduling algorithm, which
doesn't even try to do anything.  You can run it like this:
  make
  ./scheduler -dummy 0 N   # where N is some number of worker threads
All threads run right away regardless of the queue size (even zero!), and
are scheduled by the operating system.  The goal of this project is to create
scheduler implementations which are a bit more controlled and predictable.

For example, once you have completed the MP, the following should work:

  ./scheduler -fifo 1 2 3
  Main: running 2 workers on 1 queue_size for 3 iterations
  Main: detaching worker thread 3075984304
  Main: detaching worker thread 3065494448
  Main: waiting for scheduler 3086474160
  Thread 3075984304: in scheduler queue
  Thread 3075984304: loop 0
  Thread 3075984304: loop 1
  Thread 3075984304: loop 2
  Thread 3075984304: exiting
  Thread 3065494448: in scheduler queue
  Thread 3065494448: loop 0
  Thread 3065494448: loop 1
  Thread 3065494448: loop 2
  Thread 3065494448: exiting
  Scheduler: done!

The command line options used above specify:
  -fifo Use FIFO scheduling policy
  1 One thread can be in the scheduler queue at a time
  2 Create 2 worker threads
  3 Each thread runs for 3 time slices

Here's another example:

  ./scheduler -rr 10 2 3
  Main: running 2 workers on 10 queue_size for 3 iterations
  Main: detaching worker thread 3075828656
  Main: detaching worker thread 3065338800
  Main: waiting for scheduler 3086318512
  Thread 3075828656: in scheduler queue
  Thread 3065338800: in scheduler queue
  Thread 3075828656: loop 0
  Thread 3065338800: loop 0
  Thread 3075828656: loop 1
  Thread 3065338800: loop 1
  Thread 3075828656: loop 2
  Thread 3065338800: loop 2
  Thread 3075828656: exiting
  Thread 3065338800: exiting
  Scheduler: done!

The command line options used above specify:
  -rr Use Round Robin scheduling policy
  10  Ten threads can be in the scheduler queue at a time
  2 Create 2 worker threads
  3 Each thread runs for 3 time slices

Things to observe:

In both examples, the worker threads are created at the beginning of
execution.  But in the case with queue size 1, one of the threads has to
wait until the other thread exits before it can enter the scheduler queue
(the "in scheduler queue" messages).  Whereas in the case with queue size
10, both threads enter the scheduler queue immediately.

The FIFO policy would actually have basically the same behavior even with a
larger queue size; the waiting worker threads would simply be admitted to
the queue earlier.

The Round Robin scheduling policy alternates between executing the two
available threads, until they run out of work to do.


3. FILE LAYOUT
==============

The project distribution consists of the following source files:

scheduler.c
  Includes the skeleton of a scheduler (sched_proc()) and a
  parameterized dummy workload (worker_proc()).  The main() function
  accepts several parameters specifying the test workload (see
  description below).  The scheduler relies on a scheduler
  implementation (sched_impl_t) to implement the specifics of its
  scheduling policy (to be provided by you in sched_impl.[hc])

scheduler.h
  Describes the interface to which your scheduler implementation must
  adhere.  The structures containing function pointers are similar to
  Java interfaces or C++ pure virtual base classes.  This file
  declares that you must define two sched_impl_t structures,
  sched_fifo and sched_rr in another file (sched_impl.c).

dummy_impl.c
  Implements the dummy scheduling algorithm, which just lets the OS
  schedule all threads, regardless of queue size.

sched_impl.h  (define your data structures here)
  This is where you will define the data structures stored per
  scheduler instance (struct sched_queue) and per worker thread
  (struct thread_info).  This will likely include synchronization
  constructs like semaphores and mutexes, and a list of threads
  available to be scheduled.

sched_impl.c  (implement your code here)
  This is where you will define the functions implementing the core
  behavior of the scheduler, including the FIFO and Round Robin
  scheduling policies.  The only way functions defined in this file
  are made available to the main program (scheduler.c) is by placing
  function pointers in the sched_impl_t structures sched_fifo and
  sched_rr.

list.h
  Defines the basic operations on a bidirectional linked list data
  structure.  The elements of the list, of type list_elem_t, include
  a void *datum where you can store pointers to whatever kind of
  data you like.  You don't have to use this linked list library,
  but it will probably come in handy.

list.c
  Implements the linked list operations.

smp4_tests.c
testrunner.c
testrunner.h
  Test harness, defines test cases for checking your project solution.

Please take a look at the source files and familiarize yourself with how
they work.  Think about how structures containing function pointers compare
to classes and virtual methods in C++.  If you'd like to learn more, read
about the virtual function table in C++.  The struct containing function
pointers technique employed in this project is also used by C GUI libraries like
GTK+ and to define the operations of loadable modules, such as file systems,
within the Linux kernel.


4. PROGRAMMING
==============

Now you're ready to implement the core of the scheduler, including the FIFO
and Round Robin scheduling algorithms.  For this purpose, you should only
modify sched_impl.h and sched_impl.c.  Please see scheduler.h for the
descriptions of what functions you must implement.  You are free to put
whatever you want in the thread_info and sched_queue structures.  Note that
the only way that the functions you implement are made available to the main
program is through the sched_impl_t structures sched_fifo and sched_rr.  See
dummy_impl.c for a completed example of how to fill in a sched_impl_t.

Suggested approach:

4.1 Create stub versions of all of the functions you will need to implement
    in sched_impl.c, and statically initialize sched_fifo and sched_rr.

4.2 Figure out how you will implement the scheduler queue, add the
    appropriate fields to struct sched_queue, and fill in the appropriate
    queue-related operations in the functions you created in (4.1).
    Remember that we provide a doubly-linked list in list.[hc].

4.3 Implement scheduler queue admission control, so that only the requested
    number of threads can be in the scheduler queue at once.  Create the
    appropriate synchronization constructs to prevent threads not in the
    queue from executing (look at the implementation of worker threads in
    scheduler.c:worker_proc()).

4.4 Implement the queue operations for selecting the next thread to execute.
    This will be different for FIFO vs. Round Robin scheduling.

4.5 Add in synchronization constructs to make sure only the selected thread
    executes at any given time.

4.6 Fill in any gaps that might remain.

When you think you're done, you can test your program using the command
"make test".  For more thorough testing, the fifo_var and rr_var tests
accept queue_size, num_workers, and num_iterations arguments just like the
main program (but <num_iterations> is mandatory for the test case):
  ./scheduler -test fifo_var <queue_size> <num_workers> <num_iterations>
  ./scheduler -test rr_var   <queue_size> <num_workers> <num_iterations>


5. QUESTIONS
============

Q 1  What are some pros and cons of using the struct of function pointers
     approach as we did in the project to link different modules?  Does it
     significantly affect performance?  Give some examples of when you would
     and wouldn't use this approach, and why.

Q 2  Briefly describe the synchronization constructs you needed to implement
     this MP--i.e., how you mediated admission of threads to the scheduler
     queue and how you made sure only the scheduled thread would run at any
     given time.

Q 3  Why is the dummy scheduling implementation provided potentially
     unsafe (i.e. could result in invalid memory references)?  How does
     your implementation avoid this problem?

Q 4  When using the FIFO or Round Robin scheduling algorithm, can
     sched_proc() ever "miss" any threads and exit early before all threads
     have been scheduled and run to completion?  If yes, give an example; if
     no, explain why not.

Q 5  Why are the three variables in scheduler.h declared 'extern'?  What
     would happen if they were not declared 'extern'?  What would happen
     if they were not declared without the 'extern' in any file?

Q 6  Describe the behavior of exit_error() function in scheduler.c.  Why
     does exit_error() not use errno?

Q 7  Does it matter whether the call to sched_ops->wait_for_queue(queue) in
     sched_proc() actually does anything?  How would it affect correctness
     if it just returned right away?  How about performance?

Q 8  Explain how worker_proc() is able to call the appropriate
     implementation of wait_for_cpu() corresponding to the scheduling policy
     selected by the user on the command line.  Start from main() and
     briefly explain each step along the way.

Q 9  Is it possible that a worker thread would never proceed past the call to
     wa->ops->wait_for_cpu(&wa->info) when using one of the scheduling
     policies implemented in this MP?

Q 10 Explain how you would alter the program to demonstrate the "convoy"
     effect, when a large compute bound job that never yields to another
     thread slows down all other jobs in a FIFO scheduled system? See Page
     402, Stallings, the paragraph starting "Another difficulty with FCFS is
     that it tends to favor processor-bound processes over I/O bound
     processes".  Why is it difficult to show the benefits of Round Robin
     scheduling in this case using the current implementation in the machine
     problem?

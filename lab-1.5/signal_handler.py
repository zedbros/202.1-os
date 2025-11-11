import signal, sys, time

def fib():
    (m, n) = (1, 1)
    while True:
        input()
        (m, n) = (n, m + n)
        print(m)

allowedRunTime = 60
startTime = time.time()
def updateCurrentTime():
    return time.time() - startTime

def signal_handler(sig, frame):
    print("\nI caught SIGINT!")
signal.signal(signal.SIGINT, signal_handler)


while updateCurrentTime() < allowedRunTime:
    try:
        while True:
            m = input()
            print(m)
            fib()
    except:
        print("Hheheheheh")
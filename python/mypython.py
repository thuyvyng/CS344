#i always ran python3
import random
import string

#function that returns random word with designated specifications in assignment (length and ends w/ newline)
def randomWord():
    str = [random.choice(string.ascii_lowercase) for n in range(11)]
    str[10] = '\n'
    
    str1 = ""
    return str1.join(str)

#creates random word and returns it to file
def fileWrite(file):
    str = randomWord()
    file.write(str)


f1 = open("file1.txt","w+")
f2 = open("file2.txt","w+")
f3 = open("file3.txt","w+")

fileWrite(f1)
fileWrite(f2)
fileWrite(f3)


f1.close()
f2.close()
f3.close()

f1 = open("file1.txt","r+")
f2 = open("file2.txt","r+")
f3 = open("file3.txt","r+")

print(f1.read(),f2.read(),f3.read(),sep ='', end = '')

int1 = random.randint(1,42)
int2 = random.randint(1,42)
intdiv = int1 * int2

print(int1)
print(int2)
print(intdiv)

f1.close()
f2.close()
f3.close()


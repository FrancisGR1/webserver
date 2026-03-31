import random
import string

output = ''.join(random.choices(string.printable, k=random.randint(0, 4096)))
print(output)

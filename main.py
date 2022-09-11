import sys
sys.path.insert(0, "./cexts/dll")

import hallo
hallo.test()

import cuda_hallo
cuda_hallo.test()
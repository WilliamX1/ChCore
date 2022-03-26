import sys

FILE_PREFIX = sys.argv[1]
OUTPUT_TARGET = [
    [('Cap_create Pretest Ok!', 10)],
    [('[INFO] Exception type: 8', 10)],
    [('[INFO] Exception type: 8', 10)],
    [('Test: Successfully map', 2), ('Test: Successfully map for pa not 0', 3), ('handle_trans_fault: no vmr found for va 0x1!', 15)],
    [('Test: Successfully map', 2), ('Test: Successfully map for pa not 0', 3), ('Success to user land!', 15)],
    [('Test: Successfully map', 2), ('Test: Successfully map for pa not 0', 3), ('YOUR: [b]', 15), ('Back to kernel.', 10)]
]

JUDGE_RESULT = [
    ['Cap create pretest', 0], ['Bad instruction 1', 0], ['Bad instruction 2', 0], ['Fault', 0], ['User Application', 0], ['Put, Get and Exit', 0]
]

for i, assertion in enumerate(OUTPUT_TARGET):
    buf = open('{}.{}'.format(FILE_PREFIX, i + 1), 'r').read()
    caseScore = 0
    for answer, subpartScore in assertion:
        if answer in buf:
            caseScore += subpartScore
    JUDGE_RESULT[i][1] = caseScore

# Print result and get overall score
totalScore = 0
for name, score in JUDGE_RESULT:
    print('GRADE: {}: {}'.format(name, score))
    totalScore += score

print('Score: {}/{}'.format(totalScore, 100))

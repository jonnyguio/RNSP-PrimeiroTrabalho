local maxThreshold = 199
local maxBleaching = 10
local threshold = 30
local bits = 15
local bleaching = 0
local totalTimeSequencialBleaching, totalTimeSequencialNoBleaching, totalTime2ThreadsBleaching, totalTime2ThreadsNoBleaching, totalTime4ThreadsBleaching, totalTime4ThreadsNoBleaching = 0, 0, 0, 0, 0 , 0

for k = bleaching, maxBleaching, 1 do
    fileNoBleachingSequencial = io.lines("./results/" .. bits .. "-" .. k .. "-" .. 34 .. "-sequencial.txt")
    fileNoBleaching2Threads = io.lines("./results/" .. bits .. "-" .. k .. "-" .. 34 .. "-threads2.txt")
    fileNoBleaching4Threads = io.lines("./results/" .. bits .. "-" .. k .. "-" .. 34 .. "-threads4.txt")
    fileBleachingSequencial = io.lines("./results/" .. bits .. "-" .. k .. "-" .. 34 .. "-sequencial.txt")
    fileBleaching2Threads = io.lines("./results/" .. bits .. "-" .. k .. "-" .. 34 .. "-threads2.txt")
    fileBleaching4Threads = io.lines("./results/" .. bits .. "-" .. k .. "-" .. 34 .. "-threads4.txt")

    NoBleachingSequencial = fileNoBleachingSequencial()
    NoBleaching2Threads = fileNoBleaching2Threads()
    NoBleaching4Threads = fileNoBleaching4Threads()
    BleachingSequencial = fileBleachingSequencial()
    Bleaching2Threads = fileBleaching2Threads()
    Bleaching4Threads = fileBleaching4Threads()

    _, _, TimeNoBleachingSequencial = fileNoBleachingSequencial():find("(%d+) miliseconds.")
    _, _, TimeNoBleaching2Threads = fileNoBleaching2Threads():find("(%d+) miliseconds.")
    _, _, TimeNoBleaching4Threads = fileNoBleaching4Threads():find("(%d+) miliseconds.")
    _, _, TimeBleachingSequencial = fileBleachingSequencial():find("(%d+) miliseconds.")
    _, _, TimeBleaching2Threads = fileBleaching2Threads():find("(%d+) miliseconds.")
    _, _, TimeBleaching4Threads = fileBleaching4Threads():find("(%d+) miliseconds.")
    
    totalTimeSequencialNoBleaching = totalTimeSequencialNoBleaching + TimeNoBleachingSequencial
    totalTime2ThreadsNoBleaching = totalTime2ThreadsNoBleaching + TimeNoBleaching2Threads
    totalTime4ThreadsNoBleaching = totalTime4ThreadsNoBleaching + TimeNoBleaching4Threads
    totalTimeSequencialBleaching = totalTimeSequencialBleaching + TimeBleachingSequencial
    totalTime2ThreadsBleaching = totalTime2ThreadsBleaching + TimeBleaching2Threads
    totalTime4ThreadsBleaching = totalTime4ThreadsBleaching + TimeBleaching4Threads

    -- Print rights from threshold
    print(k, math.floor((NoBleachingSequencial + NoBleaching2Threads + NoBleaching4Threads) / 3), math.floor((BleachingSequencial + Bleaching2Threads + Bleaching4Threads) / 3))
end

-- Find time:
-- print(totalTimeSequencialNoBleaching / (199 - 30 + 1), totalTime2ThreadsNoBleaching / (199 - 30 + 1), totalTime4ThreadsNoBleaching / (199 - 30 + 1))
-- print(totalTimeSequencialBleaching / (199 - 30 + 1), totalTime2ThreadsBleaching / (199 - 30 + 1), totalTime4ThreadsBleaching / (199 - 30 + 1))
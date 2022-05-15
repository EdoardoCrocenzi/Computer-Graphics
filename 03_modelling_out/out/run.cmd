.\bin\ymodel --scene tests\01_terrain\terrain.json --output outs\01_terrain\terrain.json --terrain object
.\bin\ymodel --scene tests\02_displacement\displacement.json --output outs\02_displacement\displacement.json --displacement object
.\bin\ymodel --scene tests\03_hair1\hair1.json --output outs\03_hair1\hair1.json --hairbase object --hair hair
.\bin\ymodel --scene tests\03_hair2\hair2.json --output outs\03_hair2\hair2.json --hairbase object --hair hair --hairlen 0.005 --hairstr 0
.\bin\ymodel --scene tests\03_hair3\hair3.json --output outs\03_hair3\hair3.json --hairbase object --hair hair --hairlen 0.005 --hairstr 0.01
.\bin\ymodel --scene tests\03_hair4\hair4.json --output outs\03_hair4\hair4.json --hairbase object --hair hair --hairlen 0.02 --hairstr 0.001 --hairgrav 0.0005 --hairstep 8
.\bin\ymodel --scene tests\04_grass\grass.json --output outs\04_grass\grass.json --grassbase object --grass grass
.\bin\ymodel --scene tests\05_voronoise\voronoise.json --output outs\05_voronoise\voronoise.json --voronoise object
.\bin\ymodel --scene tests\06_smoothvoronoi\smoothvoronoi.json --output outs\06_smoothvoronoi\smoothvoronoi.json --smoothvoronoi object


.\bin\ymodel --scene tests\03_hair3\hair3.json --output outs\03_hair3\hair3.json --hairbase object --hair hair --hairlen 0.005 --hairstr 0.01 --densita_capelli 0.65
.\bin\ymodel --scene tests\03_hair4\hair4.json --output outs\03_hair4\hair4.json --hairbase object --hair hair --hairlen 0.02 --hairstr 0.001 --hairgrav 0.0005 --hairstep 8 --densita_capelli 0.4
.\bin\ymodel --scene tests\03_hair2\hair2.json --output outs\03_hair2\hair2.json --hairbase object --hair hair --hairlen 0.02 --hairstr 0.001 --hairgrav 0.0005 --hairstep 8 --densita_capelli 0.1
.\bin\ymodel --scene tests\04_grass\grass.json --output outs\04_grass\grass.json --grassbase object --grass grass --densita_erba 0.75
.\bin\ymodel --scene tests\04_grass\grass.json --output outs\04_grass\grass.json --grassbase object --grass grass --densita_erba 0.20
.\bin\ymodel --scene tests\04_grass\grass.json --output outs\04_grass\grass.json --grassbase object --grass grass --densita_erba 0.50



.\bin\yscene render outs\06_smoothvoronoi\smoothvoronoi.json --output out\06_smoothvoronoi.jpg --samples 256 --resolution  720
.\bin\yscene render outs\05_voronoise\voronoise.json --output out\05_voronoise.jpg --samples 256 --resolution  720
.\bin\yscene render outs\01_terrain\terrain.json --output out\01_terrain.jpg --samples 256 --resolution  720
.\bin\yscene render outs\02_displacement\displacement.json --output out\02_displacement.jpg --samples 256 --resolution  720
.\bin\yscene render outs\03_hair1\hair1.json --output out\03_hair1.jpg --samples 256 --resolution  720
.\bin\yscene render outs\03_hair2\hair2.json --output out\03_hair2.jpg --samples 256 --resolution  720
.\bin\yscene render outs\03_hair3\hair3.json --output out\03_hair3.jpg --samples 256 --resolution  720
.\bin\yscene render outs\03_hair4\hair4.json --output out\03_hair4.jpg --samples 256 --resolution  720
.\bin\yscene render outs\04_grass\grass.json --output out\04_grass.jpg --samples 256 --resolution  720 --bounces 128



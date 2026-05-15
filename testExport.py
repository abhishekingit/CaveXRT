import struct

with open("exports/CaveXRTCache/frame_000000.bin", "rb") as f:
    header = f.read(20);
    magic, version, frame, time, count = struct.unpack("IIIfI", header)
    print("Count:", count)
    positions = struct.unpack(f"{count*3}f", f.read(count * 3 * 4))
    velocities = struct.unpack(f"{count*3}f", f.read(count * 3 * 4))

    for i in range(5):
        px = positions[i*3 + 0]
        py = positions[i*3 + 1]
        pz = positions[i*3 + 2]

        vx = velocities[i*3 + 0]
        vy = velocities[i*3 + 1]
        vz = velocities[i*3 + 2]

        print(f"Particle {i}")
        print(" Pos:", px, py, pz)
        print(" Vel:", vx, vy, vz)

    

    # print("first pos:", positions[0:3])
    # print("first vel:", velocities[0:3])

    
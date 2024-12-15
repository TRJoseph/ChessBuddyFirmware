import select
import serial
import subprocess
import time

arduino_port = '/dev/ttyACM0'
baud_rate = 9600
ser = serial.Serial(arduino_port, baud_rate, timeout=1)


tigerengine_path = "/home/thomas/Desktop/ROS/TigerEngineExecutable/TigerEngine"
whiteToMove = True
moves = []

piece_types = {
    0: "Pawn",
    1: "Bishop",
    2: "Knight",
    3: "Rook",
    4: "Queen",
    5: "King"
}


def resetTigerEngine():
    moves.clear()

def send_move_to_tigerengine(tigerengine):

    formattedMoveList = f"position startpos moves {' '.join(moves)}\n"

    print(f"Sending to TigerEngine: {formattedMoveList.strip()}")

    tigerengine.stdin.write(formattedMoveList)
    tigerengine.stdin.flush()


def communicate_with_arduino(message):
    ser.write(message.encode('utf-8'))
    time.sleep(1)  # Wait a moment for Arduino to process
    response = ser.readline().decode('utf-8').strip()
    print(f"Arduino says: {response}")
    return response

def clear_initial_output(tigerengine, num_lines=5):
    for _ in range(num_lines):
        tigerengine.stdout.readline().strip()

def extract_moved_piece(splitResponse):
    try:
        # Split the response by "|", then get the part that starts with "movedPiece:"
        moved_piece_part = splitResponse[1]  # This gives you "movedPiece:0"
        
        # Now, split by ":" and get the second part which is the piece type (e.g., "0")
        moved_piece = moved_piece_part.split(":")[1]  # This gives "0"
        
        return int(moved_piece)  # Convert to integer (0 for Pawn, etc.)
    except IndexError:
        print("Error parsing response")
        return None  # Handle error case where the response is not in expected format

def run_tigerengine():

    tigerengine = subprocess.Popen(
        tigerengine_path, 
        stdin=subprocess.PIPE, 
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE,
        text=True
    )
    
    time.sleep(2)  
    
    clear_initial_output(tigerengine, num_lines=5)

    resetTigerEngine()

    return tigerengine

def main():
    tigerengine = run_tigerengine()

    while True:
        move = input("Enter your chess move (e.g., e2e4): ")
        
        if move.lower() == 'quit':
            print("Exiting game...")
            break

        moves.append(move)
        print(f"Moves so far: {moves}")

        # send the new move to TigerEngine and get the response
        send_move_to_tigerengine(tigerengine)
        
        # wait for 'readyok' before issuing the 'go' command
        while True:
            tiger_response = tigerengine.stdout.readline().strip()
            if tiger_response == "readyok":
                break
            print("waiting for a response from tigerengine before continuing...")

        # now send the 'go' in verbose mode to TigerEngine to calculate the next move
        tigerengine.stdin.write("go -v\n")
        tigerengine.stdin.flush()

        
        while True:

            rlist, _, _ = select.select([tigerengine.stdout], [], [], 5)  # timeout after 5 seconds
            if rlist:
                tigerMove = tigerengine.stdout.readline().strip()
                if tigerMove:
                    print(f"TigerEngine's move: {tigerMove}")
                    cleanedUpTigerMoveResponse = tigerMove.split("bestmove ")[1] # removes the 'bestmove' part of the response
                    splitResponse = cleanedUpTigerMoveResponse.split("|") # splits the response by the "|" delimiter
                    moveString = splitResponse[0]
                    movedPiece = extract_moved_piece(splitResponse)
                    movedPiece = piece_types.get(movedPiece, "Unknown Piece")

                    # add move response from engine to move history
                    moves.append(moveString)


                    # TODO: use switch statement here eventually
                    if "capturedPiece" in cleanedUpTigerMoveResponse:
                        capturedPiece = splitResponse[2]
                        arduinoCommandString = "docapturemove " + moveString + " " + movedPiece + " " + " " + capturedPiece
                        arduino_response = communicate_with_arduino(arduinoCommandString)
                        print(f"Response from Arduino: {arduino_response}")
                    else:
                        # MAKE ROBOT PERFORM NORMAL QUIET MOVE
                        arduinoCommandString = "doquietmove " + moveString + " " + movedPiece
                        arduino_response = communicate_with_arduino(arduinoCommandString)
                        print(f"Response from Arduino: {arduino_response}")


                    
                    break
            else:
                print("Timeout waiting for TigerEngine's move.")
                break


if __name__ == "__main__":
    main()

    
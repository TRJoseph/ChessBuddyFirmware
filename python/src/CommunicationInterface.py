import re
import select
import serial
import subprocess
import time
from serial import Serial

arduino_port = '/dev/ttyUSB0'
baud_rate = 9600
ser = serial.Serial(arduino_port, baud_rate, timeout=1)

#tigerengine_path = "/c/Users/trjos/Projects/TigerEngine/UCIEngine/bin/Debug/net8.0"

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
    # Send the message to Arduino
    ser.write(message.encode('utf-8'))
    time.sleep(1)  # Give Arduino some time to respond

    # Collect all lines of response
    responses = []
    while True:
        if ser.in_waiting > 0:  # Check if there's data available in the buffer
            line = ser.readline().decode('utf-8').strip()  # Read a line
            print(f"Arduino: {line}")  # Print each line for debugging
            responses.append(line)  # Add to the list of responses
            time.sleep(0.1)
        else:
            break

    return responses  # Return all lines as a list

def clear_initial_output(tigerengine, num_lines=5):
    for _ in range(num_lines):
        tigerengine.stdout.readline().strip()

def extract_moved_piece(pieceMovedSubstring):
    try:
        # Now, split by ":" and get the second part which is the piece type (e.g., "0")
        moved_piece = pieceMovedSubstring.split(":")[1]  # This gives "0"
        
        return int(moved_piece)  # Convert to integer (0 for Pawn, etc.)
    except IndexError:
        print("Error parsing response")
        return None  # Handle error case where the response is not in expected format

def isValidChessMove(move):
    pattern = r"^[a-h][1-8][a-h][1-8][qrnb]?$"
    return bool(re.match(pattern, move))

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
        while True:
            if ser.in_waiting > 0:
                executedMove = ser.readline().decode('utf-8').strip()
                print(f"User Executed Move: {executedMove}")
                if isValidChessMove(executedMove):
                    break
                time.sleep(0.05)

        moves.append(executedMove)
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
                    movedPiece = extract_moved_piece(splitResponse[1])
                    movedPiece = piece_types.get(movedPiece, "Unknown Piece")

                    # add move response from engine to move history
                    moves.append(moveString)


                    # TODO: use switch statement here eventually
                    if "capturedPiece" in cleanedUpTigerMoveResponse:
                        capturedPiece = extract_moved_piece(splitResponse[2])
                        capturedPiece = piece_types.get(capturedPiece, "Unknown Piece")
                        arduinoCommandString = "docapturemove " + moveString + " " + movedPiece + " " + capturedPiece
                        arduino_response = communicate_with_arduino(arduinoCommandString)
                        print(f"Response from Arduino: {arduino_response}")
                    elif "KingSideCastle" in cleanedUpTigerMoveResponse:
                        arduinoCommandString = "dokingsidecastle " + moveString
                        arduino_response = communicate_with_arduino(arduinoCommandString)
                        print(f"Response from Arduino: {arduino_response}")
                    elif "QueenSideCastle" in cleanedUpTigerMoveResponse:
                        arduinoCommandString = "doqueensidecastle" + moveString
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

    
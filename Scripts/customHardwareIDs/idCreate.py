import os
import random
import re

def generate_device_id():
    # Read the list of free words
    with open("wordList/freeWords.txt", "r") as f:
        words = f.readlines()
    
    # Strip whitespace and newline characters from each word
    words = [word.strip() for word in words]
    
    # Ensure there are at least 2 words available
    if len(words) < 2:
        print("Error: Insufficient words in freeWords.txt")
        return None

    # Choose 2 random words
    chosen_words = random.sample(words, 2)
    
    # Strip whitespace and newline characters from chosen words
    chosen_words = [word.strip() for word in chosen_words]

    # Concatenate the chosen words with a '-'
    device_id = '-'.join(chosen_words)
    filename = device_id

    # Replace spaces with underscores to meet the 32-byte requirement
    remaining_space = 26 - len(device_id)
    if remaining_space > 0:
        device_id += '_' * remaining_space

    # Remove the chosen words from the list
    words = [word for word in words if word not in chosen_words]

    # Write the concatenated words to burntIDs.txt in the wordList directory
    with open("wordList/burntIDs.txt", "a") as f:
        f.write(filename + "\n")

    # Write the ASCII bytes to a binary file in the bin directory
    bin_file_path = os.path.join("bin", f"{filename}.bin")
    with open(bin_file_path, "wb") as f:
        f.write(device_id.encode('ascii'))

    # Write the updated list of free words back to the file
    with open("wordList/freeWords.txt", "w") as f:
        f.writelines(word + '\n' for word in words)

    return bin_file_path

if __name__ == "__main__":
    bin_file_path = generate_device_id()
    if bin_file_path:
        print(f"Binary file '{bin_file_path}' created successfully.")
    else:
        print("Failed to generate device ID and binary file.")
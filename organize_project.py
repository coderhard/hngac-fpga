import os
import shutil

def move_file(filename, destination_folder):
    """Moves a file to the destination if it exists."""
    if os.path.exists(filename):
        # Create destination if it doesn't exist
        if not os.path.exists(destination_folder):
            os.makedirs(destination_folder)
        
        try:
            shutil.move(filename, os.path.join(destination_folder, filename))
            print(f"[MOVED] {filename} -> {destination_folder}/")
        except Exception as e:
            print(f"[ERROR] Could not move {filename}: {e}")
    else:
        print(f"[SKIP] {filename} not found in root (might already be moved).")

def main():
    # 1. Define the target structure
    dirs = {
        "benchmarks": "benchmarks",
        "ros_pkg": os.path.join("ros2_ws", "src", "ngac_auth"),
        "analysis": "analysis",
        "data": "data"
    }

    # 2. Create directories immediately
    for key, path in dirs.items():
        if not os.path.exists(path):
            os.makedirs(path)
            print(f"[CREATED] Directory: {path}")

    # --- MOVING FILES ---

    # Group 1: Benchmarks (Pure C++)
    move_file("ngac_benchmark.cpp", dirs["benchmarks"])
    move_file("ngac_jitter.cpp", dirs["benchmarks"])
    move_file("run_memory_profile.sh", dirs["benchmarks"])
    # Note: Only move CMakeLists.txt if it is the SPECIFIC one for benchmarks. 
    # If you have one main CMakeLists, you might need to split it manually later.

    # Group 2: ROS 2 Package
    move_file("package.xml", dirs["ros_pkg"])
    move_file("auth_node.cpp", os.path.join(dirs["ros_pkg"], "src")) # Usually goes in src/ subdir
    move_file("bad_actor_node.cpp", os.path.join(dirs["ros_pkg"], "src"))
    # Note: If CMakeLists.txt is for ROS, move it here:
    # move_file("CMakeLists.txt", dirs["ros_pkg"]) 

    # Group 3: Analysis (Python)
    move_file("plot_results.py", dirs["analysis"])
    move_file("plot_publication.py", dirs["analysis"])
    move_file("calculate_stats.py", dirs["analysis"])
    move_file("commentary_results_ngac_jitter.md", dirs["analysis"])

    # Group 4: Data (Logs)
    move_file("final_data.log", dirs["data"])
    move_file("results_ngac_jitter.txt", dirs["data"])

    print("\n------------------------------------------------")
    print("Reorganization complete.") 
    print("Run 'tree' or check your file explorer to verify.")

if __name__ == "__main__":
    # Ensure we create the specific src subdir for ROS first if moving cpp files there
    ros_src = os.path.join("ros2_ws", "src", "ngac_auth", "src")
    if not os.path.exists(ros_src):
        os.makedirs(ros_src)
        
    main()

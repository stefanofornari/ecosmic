from fastapi import FastAPI, Query, Request, HTTPException
from starlette.responses import PlainTextResponse, StreamingResponse
import uuid
import os
import aiofiles
import re

app = FastAPI()

BASE_QUEUE_DIR = "queue"

def _extract_value(line: str, key: str) -> float:
    # Ensure exact key match at the beginning of the line and handle optional units
    match = re.search(rf"^{re.escape(key)}\s*=\s*([\d\.\-E+]+)(?:\s*\[.*?\])?", line)
    if match:
        return float(match.group(1))
    raise ValueError(f"Key {key} not found in line: {line}")

def _parse_object_data(lines: list[str], object_start_keyword: str) -> tuple[list[float], list[float], str]:
    state_vector_values = [0.0] * 6
    cov_matrix_values = []
    norad_id_from_cdm = ""

    state_vector_keys = ["X", "Y", "Z", "X_DOT", "Y_DOT", "Z_DOT"]
    cov_fields = [
        "CR_R", "CT_R", "CT_T", "CN_R", "CN_T", "CN_N",
        "CRDOT_R", "CRDOT_T", "CRDOT_N", "CRDOT_RDOT",
        "CTDOT_R", "CTDOT_T", "CTDOT_N", "CTDOT_RDOT", "CTDOT_TDOT",
        "CNDOT_R", "CNDOT_T", "CNDOT_N", "CNDOT_RDOT", "CNDOT_TDOT", "CNDOT_NDOT"
    ]

    state_vector_key_map = {
        "X": 0, "Y": 1, "Z": 2,
        "X_DOT": 3, "Y_DOT": 4, "Z_DOT": 5
    }

    parsing_active = False
    for line in lines:
        stripped_line = line.strip()
        if stripped_line.startswith(object_start_keyword):
            parsing_active = True
            continue
        
        if parsing_active:
            # Stop parsing if we encounter another object block starts
            if stripped_line.startswith("OBJECT = OBJECT") and stripped_line != object_start_keyword:
                parsing_active = False
                break # Stop processing lines for this object

            # Extract OBJECT_DESIGNATOR for NORAD ID
            if stripped_line.startswith("OBJECT_DESIGNATOR"):
                match = re.search(r"OBJECT_DESIGNATOR\s*=\s*(\d+)", stripped_line)
                if match:
                    norad_id_from_cdm = match.group(1)

            # Extract state vector values
            for key in state_vector_keys:
                if stripped_line.startswith(key + " ") or stripped_line.startswith(key + "="):
                    try:
                        state_vector_values[state_vector_key_map[key]] = _extract_value(stripped_line, key)
                    except ValueError:
                        pass # Or log an error

            # Extract covariance matrix values
            for key in cov_fields:
                if stripped_line.startswith(key + " ") or stripped_line.startswith(key + "="):
                    try:
                        cov_matrix_values.append(_extract_value(stripped_line, key))
                    except ValueError:
                        pass # Or log an error
    
    return state_vector_values, cov_matrix_values, norad_id_from_cdm

@app.post("/api/cpe")
async def submit_cpe_job(owner: str | None = Query(None, alias="owner"), request: Request = Request):
    if not owner:
        raise HTTPException(status_code=403, detail="job submission not allowed")
    request_uuid = uuid.uuid4()
    file_path = os.path.join(BASE_QUEUE_DIR, "in", owner, f"{request_uuid}.cpe")

    # Check if the owner directory exists
    owner_dir = os.path.join(BASE_QUEUE_DIR, "in", owner)
    if not os.path.isdir(owner_dir):
        raise HTTPException(status_code=403, detail="job submission not allowed")

    try:
        # Read the entire request body
        body = await request.body()
        if not body:
            raise HTTPException(status_code=400, detail="No content provided")

        cdm_content = body.decode('utf-8')
        lines = cdm_content.splitlines()

        # Parse data for OBJECT1
        state_vector_obj1, cov_matrix_obj1, norad_id_obj1 = _parse_object_data(lines, "OBJECT = OBJECT1")

        # Parse data for OBJECT2
        state_vector_obj2, cov_matrix_obj2, norad_id_obj2 = _parse_object_data(lines, "OBJECT = OBJECT2")

        # Write the extracted data to the file
        async with aiofiles.open(file_path, "w") as out_file:
            await out_file.write(f"{norad_id_obj1}\n") # Norad ID for Object 1 (from query)
            await out_file.write(f"{state_vector_obj1[0]} {state_vector_obj1[1]} {state_vector_obj1[2]} {state_vector_obj1[3]} {state_vector_obj1[4]} {state_vector_obj1[5]}\n")
            await out_file.write(" ".join(map(str, cov_matrix_obj1)) + "\n")
            
            await out_file.write(f"{norad_id_obj2}\n") # Norad ID for Object 2 (from CDM)
            await out_file.write(f"{state_vector_obj2[0]} {state_vector_obj2[1]} {state_vector_obj2[2]} {state_vector_obj2[3]} {state_vector_obj2[4]} {state_vector_obj2[5]}\n")
            await out_file.write(" ".join(map(str, cov_matrix_obj2)))

        return PlainTextResponse(str(request_uuid), status_code=200)
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
    
@app.get("/api/cpe")
async def get_cpe_status(owner: str = Query(..., alias="owner"), cpe_id: str = Query(..., alias="cpe_id")):
    if not owner:
        raise HTTPException(status_code=403, detail="cpe status not allowed")                                    
    if not cpe_id:                                                                                               
        raise HTTPException(status_code=400, detail="provide the id of the cpe to check or download (cpe_id)")   

    owner_out_dir = os.path.join(BASE_QUEUE_DIR, "out", owner)

    if not os.path.isdir(owner_out_dir):          
        raise HTTPException(status_code=403, detail="cpe status not allowed")
                                                                
    file_path = os.path.join(owner_out_dir, f"{cpe_id}.cpe")                                                  
                                                                                                                 
    if not os.path.isfile(file_path):                                                                            
        raise HTTPException(status_code=404, detail="cpe not yet available")                                     
                                                                                                                 
    async def file_iterator():                                                                                   
        async with aiofiles.open(file_path, mode="rb") as f:                                                     
            while chunk := await f.read(8192):                                                                   
                yield chunk                                                                                      
                                                                                                                 
    return StreamingResponse(file_iterator(), media_type="text/plain")  
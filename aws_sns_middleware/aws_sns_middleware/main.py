from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import boto3
import config
from dotenv import load_dotenv
import os

load_dotenv()

app = FastAPI()

# Initialize SNS client
sns_client = boto3.client(
    "sns",
    aws_access_key_id=config.AWS_ACCESS_KEY_ID,
    aws_secret_access_key=config.AWS_SECRET_ACCESS_KEY,
    region_name=config.AWS_REGION,
)


class RegisterDeviceRequest(BaseModel):
    token: str


@app.post("/register")
async def register_device(request: RegisterDeviceRequest):
    try:
        # Create platform endpoint
        response = sns_client.create_platform_endpoint(
            PlatformApplicationArn=config.PLATFORM_APPLICATION_ARN, Token=request.token
        )
        endpoint_arn = response["EndpointArn"]

        # Subscribe endpoint to the SNS topic
        sns_client.subscribe(
            TopicArn=config.SNS_TOPIC_ARN, Protocol="application", Endpoint=endpoint_arn
        )

        return {
            "message": "Device registered successfully",
            "endpoint_arn": endpoint_arn,
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


if __name__ == "__main__":
    import uvicorn

    uvicorn.run(app, host="0.0.0.0", port=8000)

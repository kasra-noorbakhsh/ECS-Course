# escape=`

FROM mcr.microsoft.com/dotnet/sdk:7.0 AS builder

WORKDIR /source

# Copy csproj and restore as separate layers
COPY *.csproj .
RUN dotnet restore

# Copy everything else and publish
COPY . .
RUN dotnet publish -c release -o /app --self-contained false

# Runtime image
FROM mcr.microsoft.com/dotnet/runtime:7.0

WORKDIR /app
COPY --from=builder /app .
ENTRYPOINT ["dotnet", "Worker.dll"]

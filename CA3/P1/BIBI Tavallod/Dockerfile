FROM golang:1.22-alpine AS builder

WORKDIR /app

COPY . .

RUN go build -ldflags="-s -w" -o main

FROM scratch

COPY --from=builder /app/main /main

EXPOSE 80

ENTRYPOINT ["/main"]

//-------------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2014 Vadim Macagon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-------------------------------------------------------------------------------
#pragma once

#include "IMercurialSourceControlWorker.h"
#include "ISourceControlProvider.h"

typedef TSharedRef<class ISourceControlOperation, ESPMode::ThreadSafe> FSourceControlOperationRef;

/**
 * Executes a Mercurial command, the execution may be done on a worker thread.
 * The hard work is delegated to an IMercurialSourceControlWorker object. 
 */
class FMercurialSourceControlCommand : public FQueuedWork
{
public:
	FMercurialSourceControlCommand(
		const FSourceControlOperationRef& InOperation, 
		const FMercurialSourceControlWorkerRef& InWorker, 
		const FSourceControlOperationComplete& InCompleteDelegate = FSourceControlOperationComplete()
	);

	/** Execute the command. */
	bool DoWork();
	
	/** Return true iff the command has finished executing. */
	bool HasExecuted() const
	{
		return bExecuteProcessed != 0;
	}

	/** Update the state of any affected items after the command has executed. */
	bool UpdateStates()
	{
		check(bExecuteProcessed);

		return Worker->UpdateStates();
	}

	/** Get the result (succeeded/failed) after the command has executed. */
	ECommandResult::Type GetResult() const
	{
		check(bExecuteProcessed);

		return bCommandSuccessful ? ECommandResult::Succeeded : ECommandResult::Failed;
	}

	/** Notify that the command has finished executing. */
	void NotifyOperationComplete()
	{
		OperationCompleteDelegate.ExecuteIfBound(Operation, GetResult());
	}
	
public:
	// FQueuedWork methods
	virtual void DoThreadedWork() OVERRIDE;
	virtual void Abandon() OVERRIDE;

public:
	FSourceControlOperationRef Operation;

	/** Will be set to true if the operation is performed successfully. */
	bool bCommandSuccessful;

private:
	FMercurialSourceControlWorkerRef Worker;
	FSourceControlOperationComplete OperationCompleteDelegate;
	volatile int32 bExecuteProcessed;

	/** Is this operation being performed synchronously or asynchronously? */
	EConcurrency::Type Concurrency;
};

package com.nidium.android;

import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.io.File;
import java.util.ArrayList;

public class NMLFileAdapter extends ArrayAdapter<String> {
	Context m_context;
	int m_layoutResourceId;
	ArrayList<String> m_data = null;
	public NMLFileAdapter(Context context, int layoutResourceId, ArrayList<String> data) {
		super(context, layoutResourceId, data);
		this.m_layoutResourceId = layoutResourceId;
		this.m_context = context;
		this.m_data = data;
	}
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		View row = convertView;
		RecordHolder holder = null;

		if (row == null) {
			LayoutInflater inflater = ((Activity)this.m_context).getLayoutInflater();
			row = inflater.inflate(m_layoutResourceId, parent, false);
			holder = new RecordHolder();
			holder.fileName = (TextView)row.findViewById(R.id.file_name);
			holder.folderName = (TextView)row.findViewById(R.id.folder_name);
			row.setTag(holder);
		} else {
			holder = (RecordHolder)row.getTag();
		}

		String fullFileName = m_data.get(position);
		File nmlFile = new File(fullFileName);
		String fileName = nmlFile.getName();
		fileName = fileName.substring(0, fileName.lastIndexOf("."));
		String folderName = fullFileName.substring(Nidroid.getUserDirectory().length());
		holder.fileName.setText(fileName);
		holder.folderName.setText(folderName);

		return row;
	}

	static class RecordHolder {
		TextView fileName;
		TextView folderName;
	}
}
